#ifndef ASYNC_H
#define ASYNC_H

typedef enum { PENDING = 0, READY = 1 } poll_result_t;

typedef poll_result_t (*poll_fn_t)(void *state);

typedef struct {
  poll_fn_t poll;
  void *state;
  int completed;
} task_t;

#define MAX_TASKS 8
extern task_t tasks[MAX_TASKS];
extern int task_count;

void run_tasks(void);
void spawn_task(poll_fn_t poll, void *state);

#endif
