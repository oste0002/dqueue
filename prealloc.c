#include <stdbool.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "prealloc.h"


/* INTERNAL FUNCTION DEFINITIONS */
static int intern_realloc(prealloc_head *head);




prealloc_head *prealloc_init(const uint32_t init_cells,	const uint32_t max_cells,
    const size_t cell_size) {
	typedef char cell_data[cell_size];

	prealloc_head *head = (prealloc_head *)calloc(1,sizeof(prealloc_head));
	if (head == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	prealloc_cell *cell;

	uint32_t num_cells_x = init_cells;
	uint32_t num_cells_y;

	if ( max_cells % init_cells == 0 )
		num_cells_y = max_cells / init_cells;
	else
		num_cells_y = max_cells / init_cells + init_cells;

	head->cell_arr = (prealloc_cell **)calloc(num_cells_y,
      sizeof(prealloc_cell *));
	if (head->cell_arr == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	head->cell_arr[0] = (prealloc_cell *)calloc(num_cells_x,
      sizeof(prealloc_cell));
	if (head->cell_arr[0] == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	head->data = (void **)calloc(num_cells_y, sizeof(cell_data *));
	if (head->data == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	head->data[0] = (void *)calloc(num_cells_x, sizeof(cell_data));
	if (head->data[0] == NULL) {
		fprintf(stderr,"prealloc: Allocation failed!\n");
		exit(EXIT_FAILURE);	}

	for (unsigned int i=0;i<init_cells;i++) {
		cell = &(head->cell_arr[0][i]);
		cell->data = &((cell_data **)head->data)[0][i];	}

	head->full_next[0] = 0;
	head->full_next[1] = 0;
	head->num_cells = 0;
	head->avail_cell[0] = -1U;
	head->avail_cell[1] = -1U;
	head->init_cells = num_cells_x;
	head->alloc_cells = num_cells_x;
	head->max_cells = max_cells;
	head->cell_size = cell_size;
	return(head);
}


prealloc_cell *prealloc_new(prealloc_head *head) {
	prealloc_cell *cell;

	if ( head->avail_cell[0] == -1U) {
		if (head->full_next[0] > head->init_cells -1) {
			if (intern_realloc(head) != 0)
				return(NULL);	}
		cell = &(head->cell_arr[head->full_next[1]][head->full_next[0]]);
		cell->place[0] = head->full_next[0];
		cell->place[1] = head->full_next[1];
		head->full_next[0]++;	}

	else {
		cell = &(head->cell_arr[head->avail_cell[1]][head->avail_cell[0]]);
		cell->place[0] = head->avail_cell[0];
		cell->place[1] = head->avail_cell[1];
		head->avail_cell[0] =	cell->next_avail[0];
		head->avail_cell[1] =	cell->next_avail[1]; }

	cell->is_used = true;
	head->num_cells++;
	return(cell);
}


void prealloc_del(prealloc_head *head, prealloc_cell *cell) {

	cell->next_avail[0] = head->avail_cell[0];
	cell->next_avail[1] = head->avail_cell[1];
	head->avail_cell[0] = cell->place[0];
	head->avail_cell[1] = cell->place[1];

	memset(cell->data,0,head->cell_size);

	cell->is_used = false;
	head->num_cells--;
}


void prealloc_destroy(prealloc_head *head) {

	unsigned int i;

	for (i=0; i <= head->full_next[1]; i++) {
		free(head->cell_arr[i]);
		free(head->data[i]); }

	free(head->cell_arr);
	free(head->data);
	free(head);
}


static int intern_realloc(prealloc_head *head) {

	typedef char cell_data[head->cell_size];

	if (head->max_cells < head->alloc_cells + head->init_cells)
		return(-1);

	prealloc_cell *cell = {0};

	head->full_next[0] = 0;
	head->full_next[1]++;

	head->cell_arr[head->full_next[1]] =
		(prealloc_cell *)calloc(head->init_cells, sizeof(prealloc_cell));
	if (head->cell_arr[head->full_next[1]] == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	head->data[head->full_next[1]] =
		(void *)calloc(head->init_cells, head->cell_size);

	if (head->data[head->full_next[1]] == NULL) {
		perror("prealloc, calloc");
		exit(EXIT_FAILURE);	}

	for (unsigned int i=0;i<head->init_cells;i++) {
		cell = &(head->cell_arr[head->full_next[1]][i]);
		cell->data = &((cell_data **)head->data)[head->full_next[1]][i]; }

	head->alloc_cells = head->alloc_cells + head->init_cells;
	return(0);
}

