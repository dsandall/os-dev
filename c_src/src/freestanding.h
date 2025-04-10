#ifndef FREESTANDING_H
#define FREESTANDING_H

// https://wiki.osdev.org/C_Library#Freestanding_and_Hosted
// #include <float.h>
// #include <iso646.h>
#include <limits.h>
// #include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include <stdnoreturn.h>
//

typedef struct position {
  int x;
  int y;
} position_t;

#endif // FREESTANDING_H
