#ifndef DQUEUE_H
#define DQUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include "prealloc.h"

#define PREALLOC_ALLOC_SIZE 100
#define PREALLOC_MAX_SIZE 10000



typedef struct Dqueue_Head dqueue_head;
typedef struct Dqueue_Link dqueue_link;
typedef struct Dqueue_Prealloc dqueue_prealloc;
typedef bool *dqueue_identifier;

struct Dqueue_Prealloc {
  pthread_mutex_t *p_lock;
  prealloc_cell *p_cell;
  prealloc_head *p_head;
};

/* 'is_head' must be first item in structure for dqueue_identifier
 * to work. For Dqueue_Link, is_head is initially false
 * and used for identification, but it may be true when
 * denoting the start of a subqueue. */

/* The origin of the link assign the number of destination
 * threads to 'num_poppers', before the link. When a thread
 * pops the link, also subtracts 1 from num_poppers. When
 * (num_poppers == 0) the origin of the link may reuse link. */
struct Dqueue_Link {
  bool is_head;
  dqueue_prealloc prealloc;
  struct Dqueue_Link *next;
  pthread_mutex_t next_lock;
  unsigned int num_poppers;
  pthread_mutex_t num_poppers_lock;
  //prealloc_cell *p_cell;
  size_t data_size;
  const void *data;
} __attribute__((packed));

struct Dqueue_Head{
  bool is_head;
  prealloc_head *p_head;
  pthread_mutex_t p_head_lock;
  dqueue_link *push;
  dqueue_link *pop;
  pthread_mutex_t push_lock;
  pthread_mutex_t pop_lock;
} __attribute__((packed));



/* DQUEUE_INIT - Initializes a queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be initialized.
 *
 *  Return:  0: Success
 *          !0: Failure
 */
int dqueue_init(dqueue_head *head);



/* DQUEUE_PUSH - Push data into the queue.
 *
 *  dqueue_head *head     - A pointer to the queue that will be increased.
 *  int32_t *num_poppers  - The number of threads that will use this link.
 *  void *data            - A pointer to the data that will be inserted.
 *  ssize_t data_size     - Size of the inserted data.
 *
 *  Return:  0: Success
 *          !0: Failure
 */
int dqueue_push(dqueue_head *head, int32_t num_poppers, void *data,
    size_t data_size);



/* DQUEUE_PUSH_QUEUE - Push a queue into another queue.
 *
 *  dqueue_head *head       - A pointer to the queue that will be increased.
 *  dqueue_head *push_head  - A pointer to the queue that will be inserted.
 *
 *  Return:  0: Success
 *          !0: Failure
 */
int dqueue_push_queue(dqueue_head *head, dqueue_head *push_head);



/* DQUEUE_POP - Pop a link from the queue. If there is no other thread that will
 *              be using this link, the queue link is deleted
 *
 *  dqueue_head *head     - A pointer to the queue that will be reduced.
 *  dqueue_link *data     - A pointer to the data that will be retreived.
 *
 * Return:  + Size of the retreived data.
 *          - Size of the retreived data if the link also is deleted.
 *          0 If the queue is empty.
 */
ssize_t dqueue_pop(dqueue_head *head, void *data);


/* DQUEUE_DESTROY - Destroys a queue.
 *
 *  dqueue_head *head     - A pointer to a queue that will be destroyed.
 */
void dqueue_destroy(dqueue_head *head);


/* DQUEUE_IS_EMPTY - Checks if a dqueue is empty
 *
 *  dqueue_head *head     - A pointer to a queue that will be checked.
 *
 * Return:  true  - Queue is empty
 *          false - Queue is not empty
 */
#define dqueue_is_empty(head) (head->pop == NULL ? true : false)


/* DQUEUE_LAST_IS_POPPED - Checks if the last link of a dqueue is all popped
 *                         (ready to free link data).
 *
 *  dqueue_head *head     - A pointer to a queue that will be checked.
 *
 * Return:  true  - Last link is all popped
 *          false - Last link is not all popped or queue is empty
 */
#define dqueue_last_is_popped(head)    \
((head->pop == NULL) && (head->pop->num_poppers == 0) ? true : false)

#endif
