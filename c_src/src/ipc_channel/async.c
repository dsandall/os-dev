#include "async.h"

task_t tasks[MAX_TASKS];
int task_count = 0;

void spawn_task(run_fn_t poll, void *state) {
  if (task_count < MAX_TASKS) {
    tasks[task_count++] = (task_t){.run = poll, .state = state, .completed = 0};
  }
}

void run_tasks(void) {
  for (int i = 0; i < task_count; ++i) {
    if (!tasks[i].completed) {
      run_result_t res = tasks[i].run(tasks[i].state);
      if (res == READY) {
        tasks[i].completed = 1;
      }
    }
  }
}
