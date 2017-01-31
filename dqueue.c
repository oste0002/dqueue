#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

  prealloc_cell *p_cell = prealloc_new(head->p_head);
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
  pthread_mutex_lock(&head->push_lock);
  pthread_mutex_lock(&head->push->next_lock);

  if (head->push != NULL)
    head->push->next = push_link;
  else
    head->pop = push_link;
  head->push = push_link;

  pthread_mutex_unlock(&head->push_lock);
  pthread_mutex_unlock(&head->push->next_lock);

  return 0;
}


int dqueue_push_queue(dqueue_head *head, dqueue_head *push_head) {

  /* It is important to always lock the head before the link to avoid
   * situations where two calling functions block each other */
  pthread_mutex_lock(&head->push_lock);
  pthread_mutex_lock(&head->push->next_lock);

  head->push->next = push_head->pop;
  head->push = push_head->push;
  head->push->is_head = true;

  pthread_mutex_unlock(&head->push_lock);
  pthread_mutex_unlock(&head->push->next_lock);

  free(push_head);

  return 0;
}


size_t dqueue_pop(dqueue_head *head, void *data) {
  size_t data_size;
  bool del_link = false;
  dqueue_link *pop_link = head->pop;

  pthread_mutex_lock(&head->pop_lock);

  if (pop_link == NULL) {
    pthread_mutex_unlock(&head->pop_lock);
    return 0; }

  // Copy data
  data_size = pop_link->data_size;
  memcpy(data, pop_link->data, data_size);

  // Reduce queue
  pthread_mutex_lock(&head->pop->num_poppers_lock);
  pop_link->num_poppers--;

  if ( pop_link->num_poppers <= 0 )
    del_link = true;

  pthread_mutex_unlock(&head->pop->num_poppers_lock);


  head->pop = pop_link->next;
  pthread_mutex_unlock(&head->pop_lock);

  if ( del_link == true ) {
    pthread_mutex_lock(pop_link->prealloc.p_lock);
    prealloc_del(pop_link->prealloc.p_head, pop_link->prealloc.p_cell);
    pthread_mutex_unlock(pop_link->prealloc.p_lock);

    // If link is the head of an inserted sub-queue, 'is_head' is assigned false
    // The caller of the inserted sub-queue may check for if
    // (head->push == false). If it is, the caller may free the queue with 
    // 'dqueue_destroy' and also free eventual data used for storage.
    if ( pop_link->is_head == true )
      pop_link->is_head = false;
  }

  return data_size;
}


void dqueue_destroy(dqueue_head *head) {
  prealloc_destroy(head->p_head);
}


