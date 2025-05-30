#ifndef SchedulerSlot_H
#define SchedulerSlot_H

#include "coop.h"
#include "freestanding.h"

typedef struct SchedulerSlot {
  Process *proc; // WARN: must be first for ASM
  struct SchedulerSlot *next;
  struct SchedulerSlot *prev;
} SchedulerSlot;

extern SchedulerSlot *scheduler_current;
extern SchedulerSlot *scheduler_on_deck;

static void insert_end(SchedulerSlot **ptr_to_list, SchedulerSlot *item) {
  ASSERT(ptr_to_list != NULL);
  ASSERT(item != NULL);
  ASSERT(item->next == item && item->prev == item); // not already in a list

  breakpoint();
  SchedulerSlot *list = *ptr_to_list;
  if (list == NULL) {
    // item is now the only element in the list
    tracek("list is empty, adding\n");
    *ptr_to_list = item;
    return;
  }

  SchedulerSlot *temp = list->prev;

  ASSERT(temp != NULL);
  ASSERT(temp->next == list);
  ASSERT(list->prev == temp);

  list->prev = item;
  item->next = list;
  item->prev = temp;
  temp->next = item;
}

static SchedulerSlot *separate_from(SchedulerSlot *item) {
  ASSERT(item != NULL);
  ASSERT(item->next != NULL);
  ASSERT(item->prev != NULL);
  ASSERT(item->next->prev == item);
  ASSERT(item->prev->next == item);

  SchedulerSlot *prevprev = item->prev;
  SchedulerSlot *next = item->next;

  if (item == next)
    return NULL;

  prevprev->next = next;
  next->prev = prevprev;

  item->next = item;
  item->prev = item;

  return next;
}

void setOnDeck(SchedulerSlot *);

#endif
