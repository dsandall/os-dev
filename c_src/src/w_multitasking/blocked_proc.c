#include "coop.h"

typedef struct {

} ProcessQueue;

void PROC_block_on(ProcessQueue *pq, int enable_ints) {};
void PROC_unblock_all(ProcessQueue *pq) {};
void PROC_unblock_head(ProcessQueue *pq) {};
void PROC_init_queue(ProcessQueue *pq) {};
