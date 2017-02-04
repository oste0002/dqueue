#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

#include "dqueue.h"
#include "prealloc.h"



int dqueue_init(dqueue_head *head) {

  head->is_head = true;
  head->p_head =
    prealloc_init(PREALLOC_ALLOC_SIZE, PREALLOC_MAX_SIZE, sizeof(dqueue_link));
  head->p_head_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  head->push = NULL;
  head->pop = NULL;
  head->push_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  head->pop_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

  return 0;
}


int dqueue_push(dqueue_head *head, int32_t num_poppers, void *data,
    size_t data_size) {
    prealloc_cell *p_cell;

  if ( (p_cell = prealloc_new(head->p_head)) == NULL)
    return 1;
  dqueue_link *push_link = (dqueue_link *) prealloc_memget(p_cell);
  push_link->prealloc.p_cell = p_cell;
  push_link->prealloc.p_head = head->p_head;
  push_link->prealloc.p_lock = &head->p_head_lock;
  push_link->is_head = false;
  push_link->next_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  push_link->data_size = data_size;
  push_link->data = data;
  push_link->num_poppers = num_poppers;
  push_link->num_poppers_lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

  /* It is important to always lock the head before the link to avoid
   * situations where two calling functions block each other */
  if ((errno = pthread_mutex_lock(&head->push_lock)) != 0)
    perror("Lock mutex");
  if ((errno = pthread_mutex_lock(&head->push->next_lock)) != 0)
    perror("Lock mutex");

  if (head->push != NULL)
    head->push->next = push_link;
  else
    head->pop = push_link;
  head->push = push_link;

  if ((errno = pthread_mutex_unlock(&head->push_lock)) != 0)
    perror("Unlock mutex");
  if ((errno = pthread_mutex_unlock(&head->push->next_lock)) != 0)
    perror("Unlock mutex");

  return 0;
}


int dqueue_push_queue(dqueue_head *head, dqueue_head *push_head) {

  /* It is important to always lock the head before the link to avoid
   * situations where two calling functions block each other */
  if ((errno = pthread_mutex_lock(&head->push_lock)) != 0)
    perror("Lock mutex");
  if ((errno = pthread_mutex_lock(&head->push->next_lock)) != 0)
    perror("Lock mutex");

  head->push->next = push_head->pop;
  head->push = push_head->push;
  head->push->is_head = true;

  if ((errno = pthread_mutex_unlock(&head->push_lock)) != 0)
    perror("Unlock mutex");
  if ((errno = pthread_mutex_unlock(&head->push->next_lock)) != 0)
    perror("Unlock mutex");

  free(push_head);

  return 0;
}


ssize_t dqueue_pop(dqueue_head *head, void *data) {
  size_t data_size;
  bool del_link = false;
  dqueue_link *pop_link = head->pop;

  if ((errno = pthread_mutex_lock(&head->pop_lock)) != 0)
    perror("Lock mutex");

  // Return if queue is empty
  if (pop_link == NULL) {
    if ((errno = pthread_mutex_unlock(&head->pop_lock)) != 0)
      perror("Unlock mutex");
    return 0; }

  // Copy data
  data_size = pop_link->data_size;
  memcpy(data, &pop_link->data, sizeof(void *));

  // Reduce queue and mark link for deletion if no other thread will be using it
  if ((errno = pthread_mutex_lock(&head->pop->num_poppers_lock)) != 0)
    perror("Lock mutex");
  pop_link->num_poppers--;
  if ( pop_link->num_poppers <= 0 )
    del_link = true;
  if ((errno = pthread_mutex_unlock(&head->pop->num_poppers_lock)) != 0)
    perror("Unlock mutex");
  head->pop = pop_link->next;
  if (head->pop == NULL)
    head->push = NULL;
  if ((errno = pthread_mutex_unlock(&head->pop_lock)) != 0)
    perror("Unlock mutex");

  // Delete link if it is marked for deletion. (The reason for not deleting it
  // directly above is that the prealloc lock has to be locked, and it is global
  // so it may hold. It is then better to unlock other locks to speed up for
  // other threads.)
  if ( del_link == true ) {
    if ((errno = pthread_mutex_lock(pop_link->prealloc.p_lock)) != 0)
      perror("Lock mutex");

    prealloc_del(pop_link->prealloc.p_head, pop_link->prealloc.p_cell);

    if ((errno = pthread_mutex_unlock(pop_link->prealloc.p_lock)) != 0)
      perror("Unlock mutex");

    return(-data_size);
  }
  return data_size;
}

