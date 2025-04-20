#include "async.h"
#include "freestanding.h"
#include "printer.h"

task_t tasks[MAX_TASKS];
int task_count = 0;

void spawn_task(run_fn_t poll, void *state, run_fn_t init_fn) {

  // if there is a free task //WARN: (dead tasks occupy space)
  if (task_count < MAX_TASKS) {
    // then increment the tasks, and initialize with
    // your function and state pointers

    if (poll != NULL)
      tasks[task_count++] = (task_t){.run = poll, .state = state, .dead = 0};

    if (init_fn != NULL)
      init_fn(state);

  } else {
    debugk("No task slots remaining\n");
    ERR_LOOP();
  }
}

void run_tasks(void) {
  // for all allocated tasks,
  for (int i = 0; i < task_count; ++i) {
    // if its not dead,
    if (!tasks[i].dead) {
      // run the task function with it's given state pointer passed to it
      run_result_t res = tasks[i].run(tasks[i].state);
      // mark dead if dead
      if (res == DEAD) {
        tasks[i].dead = 1;
      }
    }
  }
}
