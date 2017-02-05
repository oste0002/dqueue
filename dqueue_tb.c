#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>

#include <signal.h>

#include "dqueue.h"
#include "prealloc.h"



typedef struct ThreadCtrl thread_ctrl;

struct ThreadCtrl {
  char id;
  struct timespec loop_tim;
  dqueue_head *q_head;
};



static void thread_cleanup(void *arg);
void *thread_routine(void *arg_in);



static volatile bool run = true;
void sigint_handler() {
  run = false;
}


const int num_names = 20;
const char *names[20] = {"Oskar", "Petra", "Ellen", "Johannes", "Nicolas",
  "Anna", "Freddy", "Bengt", "Barbro", "Stig", "Sara", "Bertil", "Malin",
  "Magnus","Johan", "Lena", "Kerstin", "Håkan", "Veronica", "Alexandra"};




int main() {
  pthread_t thread_1 ,thread_2, thread_3;
  struct timespec tim_3s = {.tv_sec = 3, .tv_nsec = 0};               // 3000 ms
  struct timeval tod;
  unsigned int rand_seed;
  int ni;


  // Start signal handler
  signal(SIGINT, sigint_handler);


  // Initialize timers
  struct timespec loop_tim_1  = {.tv_sec = 0, .tv_nsec = 050000000};  // 50 ms
  struct timespec loop_tim_2  = {.tv_sec = 0, .tv_nsec = 050000000};  // 50 ms
  struct timespec loop_tim_3  = {.tv_sec = 0, .tv_nsec = 050000000};  // 50 ms


  // Initialize queues
  dqueue_head q_head_main, q_head_1, q_head_2, q_head_3;
  if ( dqueue_init(&q_head_main) != 0 ) {
    fprintf(stderr, "dqueue_init: Initialization failed\n");
    exit(EXIT_FAILURE); }
  if ( dqueue_init(&q_head_1) != 0 ) {
    fprintf(stderr, "dqueue_init: Initialization failed\n");
    dqueue_destroy(&q_head_main);
    exit(EXIT_FAILURE); }
  if ( dqueue_init(&q_head_2) != 0 ) {
    fprintf(stderr, "dqueue_init: Initialization failed\n");
    dqueue_destroy(&q_head_1);
    dqueue_destroy(&q_head_main);
    exit(EXIT_FAILURE); }
  if ( dqueue_init(&q_head_3) != 0 ) {
    fprintf(stderr, "dqueue_init: Initialization failed\n");
    dqueue_destroy(&q_head_2);
    dqueue_destroy(&q_head_1);
    dqueue_destroy(&q_head_main);
    exit(EXIT_FAILURE); }


  // Initialize thread control structures
  thread_ctrl ctrl_1= {.id='A', .loop_tim=loop_tim_1, .q_head=&q_head_1};
  thread_ctrl ctrl_2= {.id='B', .loop_tim=loop_tim_2, .q_head=&q_head_2};
  thread_ctrl ctrl_3= {.id='C', .loop_tim=loop_tim_3, .q_head=&q_head_3};


  // Create threads
  if ((errno = pthread_create(&thread_1, NULL, &thread_routine, &ctrl_1)) != 0)
    perror("pthread_create");
  if ((errno = pthread_create(&thread_2, NULL, &thread_routine, &ctrl_2)) != 0)
    perror("pthread_create");
  if ((errno = pthread_create(&thread_3, NULL, &thread_routine, &ctrl_3)) != 0)
    perror("pthread_create");


  // Generate random seed
  gettimeofday(&tod, NULL);
  rand_seed = (((tod.tv_usec & 0xffff0000) >> 16) | (tod.tv_usec & 0x0000ffff));
  ni = num_names%rand_r(&rand_seed);


  // Get memory




  while( run == true ) {
    ni = num_names % rand_seed;







    rand_seed = rand_r(&rand_seed);
    nanosleep(&tim_3s, NULL);
  }


  // Cancel threads
  pthread_cancel(thread_1);
  pthread_cancel(thread_2);
  pthread_cancel(thread_3);


  // Destroy queues
  dqueue_destroy(&q_head_1);
  dqueue_destroy(&q_head_2);
  dqueue_destroy(&q_head_3);

  exit(EXIT_SUCCESS);
}


static void thread_cleanup(void *arg_in) {
//thread_ctrl *ctrl_p = (thread_ctrl *)arg_in;
  arg_in = arg_in;
}


void *thread_routine(void *arg_in) {
  thread_ctrl *ctrl = (thread_ctrl *) arg_in;
  prealloc_cell p_cell;
  ssize_t pop_siz;

  pthread_cleanup_push(thread_cleanup, ctrl);


  for (;;) {
    if ( (pop_siz = dqueue_pop(ctrl->q_head, &p_cell)) == 0 ) {
      nanosleep(&ctrl->loop_tim, NULL);
      continue; }








    if (pop_siz < 0)
      //prealloc_del

    nanosleep(&ctrl->loop_tim, NULL);
  }

  pthread_cleanup_pop(1);
  return NULL;
}


