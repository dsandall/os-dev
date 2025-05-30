#include "async.h"

extern async_run_result_t ps2_rx_task(void *s);

extern async_run_result_t vga_task(void *initial_state);
extern async_run_result_t vga_task_init(void *initial_state);

extern async_run_result_t hw_int_task(void *initial_state);
extern async_run_result_t hw_int_task_init(void *initial_state);

extern async_run_result_t hw_serial_task(void *initial_state);
extern async_run_result_t hw_serial_init(void *initial_state);

extern void printchar_serialtask(char c);
