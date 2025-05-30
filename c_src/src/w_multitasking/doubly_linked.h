#ifndef SchedulerSlot_H
#define SchedulerSlot_H

#include "coop.h"

typedef struct SchedulerSlot {
  Process *proc; // WARN: must be first for ASM
  struct SchedulerSlot *next;
  struct SchedulerSlot *prev;
} SchedulerSlot;

static void insert(SchedulerSlot *list, SchedulerSlot *item) {
  ASSERT(list != NULL);
  ASSERT(item != NULL);
  ASSERT(item->next == NULL && item->prev == NULL); // not already in a list

  SchedulerSlot *temp = list->next;

  ASSERT(temp != NULL);
  ASSERT(temp->prev == list);
  ASSERT(list->next == temp);

  list->next = item;
  item->prev = list;
  item->next = temp;
  temp->prev = item;
}

static SchedulerSlot *separate_from(SchedulerSlot *item) {
  ASSERT(item != NULL);
  ASSERT(item->next != NULL);
  ASSERT(item->prev != NULL);
  ASSERT(item->next->prev == item);
  ASSERT(item->prev->next == item);

  SchedulerSlot *prev = item->prev;
  SchedulerSlot *next = item->next;

  prev->next = next;
  next->prev = prev;

  item->next = NULL;
  item->prev = NULL;

  if (item == next)
    return NULL;

  return next;
}

#endif
