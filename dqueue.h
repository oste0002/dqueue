#ifndef DQUEUE_H
#define DQUEUE_H

#include <pthread.h>
#include <stdbool.h>


typedef struct Dqueue_Head dqueue_head;
typedef struct Dqueue_Link dqueue_link;
typedef bool *dqueue_identifier;


struct Dqueue_Link {
  bool is_head;     // Must be first item in struct
  struct Dqueue_Link *next;
  pthread_mutex_t mutex_next;
  size_t data_size;
  void *data;
} __attribute__((packed));

struct Dqueue_Head{
  bool is_head;     // Must be first item in struct
  bool is_blocked;
  dqueue_link *push;
  dqueue_link *pop;
  pthread_mutex_t mutex_push;
  pthread_mutex_t mutex_pop;
};






/* DQUEUE_INIT - Initializes a queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be initialized.
 */
dqueue_head *dqueue_init();


/* DQUEUE_PUSH - Push data into the queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be increased.
 *  void *data            - A pointer to the data that will be inserted.
 *  ssize_t data_size     - Size of the inserted data.
 */
int dqueue_push(dqueue_head *head, void *data, size_t data_size);



/* DQUEUE_PUSH_QUEUE - Push a queue into another queue.
 *
 *  dqueue_head *head       - A pointer to the queue that will be increased.
 *  dqueue_head *push_head   - A pointer to the queue that will be inserted.
 */
int dqueue_push_queue(dqueue_head *head, dqueue_head *push_head);



/* DQUEUE_POP - Pop a link from the queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be reduced.
 *  dqueue_link *data     - A pointer to the data that will be retreived.
 *  ssize_t data_size     - Size of the retreived data.
 */
int dqueue_pop(dqueue_head *head, void *data, size_t *data_size);


/* DQUEUE_DESTROY - Destroys a queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be destroyed.
 */
void dqueue_destroy(dqueue_head *head);

#endif
