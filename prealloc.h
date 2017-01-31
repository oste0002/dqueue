#ifndef PREALLOC_H
#define PREALLOC_H

#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>


typedef struct Prealloc_Cell {
	uint32_t place[2];
	uint32_t next_avail[2];
	bool is_used;
	void *data;
} prealloc_cell;


typedef struct Prealloc_Head {
	prealloc_cell **cell_arr;
	void **data;
	uint32_t full_next[2];
	uint32_t avail_cell[2];
	uint32_t init_cells;
	uint32_t alloc_cells;
	uint32_t max_cells;
	uint32_t num_cells;
	size_t cell_size;
} prealloc_head;


/* PREALLOC_INIT - Initialize prealloc array
 *
 *  uint32_t alloc_size   - Size of one allocation set. This size is the size
 *                          of both the first allocatin and a reallocation.
 *  uint32_t max_size     - Maximum size allowed in the prealloc array.
 *  size_t cell_size      - Size of one cell in the array.
 *
 * Return:  A pointer to the prealloc array.
 */
prealloc_head *prealloc_init(const uint32_t alloc_size, const uint32_t max_size,
		const size_t cell_size);


/* PREALLOC_NEW - Request a new memory cell.
 *
 * prealloc_head *head    - A pointer to the prealloc array head, from which the
 *                          cell will be requested.
 *
 * Return:  A pointer to the cell that has been made available.
 */
prealloc_cell *prealloc_new(prealloc_head *head);


/* PREALLOC_MEMGET - Request memory segment from a cell. When writing to this
 *                   memory segment, the user is responsible for not exceeding
 *                   its size. The size was specified on initialization
 *                   'PREALLOC_INIT()'.
 *  prealloc_cell *cell   - A pointer to the cell, from which the memory will
 *                          be requsted.
 *
 * Return:  A pointer to the memory segment.
 */
void *prealloc_memget(const prealloc_cell *cell);


/* PREALLOC_DEL - Delete a cell from the prealloc array.
 *
 *  prealloc_head *head   - A pointer to the prealloc array head, from which the
 *                          cell will be deleted.
 *  prealloc_cell *cell   - A pointer to the cell that will be deleted.
 */
void prealloc_del(prealloc_head *head, prealloc_cell *cell);


/* PREALLOC_DESTROY - Destroy a prealloc array. This function should be called
 *                    when the prealloc array will not be used any more.
 *
 *  prealloc_head *head   - A pointer to the prealloc array head, that will be
 *                          deleted.
 */
void prealloc_destroy(prealloc_head *head);


#endif
