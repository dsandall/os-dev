#include "freestanding.h"

//
// Memory Functions
//
void *memset(void *dst, int c, size_t n) {
  uint8_t *p = dst;
  while (n--) {
    *p++ = (uint8_t)c;
  }
  return dst;
}

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *d = dest;
  const uint8_t *s = src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

//
// String Functions
//
size_t strlen(const char *s) {
  const char *p = s;
  while (*p)
    p++;
  return p - s;
}

char *strcpy(char *dest, const char *src) {
  char *d = dest;
  while ((*d++ = *src++))
    ;
  return dest;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return (int)((unsigned char)*s1 - (unsigned char)*s2);
}

const char *strchr(const char *s, int c) {
  char ch = (char)c;
  while (*s) {
    if (*s == ch)
      return s;
    s++;
  }
  return (ch == '\0') ? s : 0;
}

// extern char *strdup(const char *s);
// WARN: No Malloc
// TODO: Malloc
