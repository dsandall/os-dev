#ifndef ASYNC_H
#define ASYNC_H

typedef enum {
  PENDING = 0,
  DEAD = 1,
  DATA = 2
} async_run_result_t; // returned by poll_fn_t

typedef async_run_result_t (*run_fn_t)(
    void *state); // poll_fn_t: represents a single task that returns a result

typedef struct {
  run_fn_t run; // the task
  void *state;  // some task specific shared state pointer
  int dead;
} async_task_t;

#define MAX_TASKS 8
extern async_task_t tasks[MAX_TASKS];
extern int task_count;

void run_tasks(void);
void spawn_task(run_fn_t poll, void *state, run_fn_t init_fn);

#endif
