#ifndef ASYNC_H
#define ASYNC_H

typedef enum { PENDING = 0, READY = 1 } run_result_t; // returned by poll_fn_t

typedef run_result_t (*run_fn_t)(
    void *state); // poll_fn_t: represents a single task that returns a result

typedef struct {
  run_fn_t run; // the task
  void *state;  // some task specific shared state pointer
  int completed;
} task_t;

#define MAX_TASKS 8
extern task_t tasks[MAX_TASKS];
extern int task_count;

void run_tasks(void);
void spawn_task(run_fn_t run_func, void *state);

#endif
