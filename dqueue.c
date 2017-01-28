#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dqueue.h"



dqueue_head *dqueue_init() {
  dqueue_head *head = (dqueue_head *)calloc(1,sizeof(dqueue_head));

  head->is_head = true;
  head->is_blocked = false;
  head->push = NULL;
  head->pop = NULL;
  head->mutex_push = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  head->mutex_pop = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;

  return head;
}


int dqueue_push(dqueue_head *head, void *data, size_t data_size) {

    dqueue_link *push_link = (dqueue_link *)calloc(1,sizeof(dqueue_link));
    push_link->is_head = false;
    push_link->mutex_next = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    push_link->data_size = data_size;
    push_link->data = data;

    /* It is important to always lock head before the link to avoid
     * situations where two calling functions block each other */
    pthread_mutex_lock(&head->mutex_push);
    pthread_mutex_lock(&head->push->mutex_next);

    if (head->push != NULL)
      head->push->next = push_link;
    else
      head->pop = push_link;

    head->push = push_link;

    pthread_mutex_unlock(&head->mutex_push);
    pthread_mutex_unlock(&head->push->mutex_next);

  return 0;
}


int dqueue_push_queue(dqueue_head *head, dqueue_head *push_head) {

    /* It is important to always lock head before the link to avoid
     * situations where two calling functions block each other */
    pthread_mutex_lock(&head->mutex_push);
    pthread_mutex_lock(&head->push->mutex_next);

    head->push->next = push_head->pop;
    head->push = push_head->push;

    pthread_mutex_unlock(&head->mutex_push);
    pthread_mutex_unlock(&head->push->mutex_next);

    free(push_head);

  return 0;
}


int dqueue_pop(dqueue_head *head, void *data, size_t *data_size) {

    pthread_mutex_lock(&head->mutex_pop);

    data = head->pop->data;
    *data_size = head->pop->data_size;
    free(head->pop);
    head->pop = head->pop->next;

    pthread_mutex_unlock(&head->mutex_pop);



  return 0;
}


void dqueue_destroy(dqueue_head *head) {
  free(head);
}


