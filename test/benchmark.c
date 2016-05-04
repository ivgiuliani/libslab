#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "queue.h"
#include "slab.h"

typedef struct __obj {
  char padding[8];
  int value1;
  int value2;
} obj;

void
measure(const char *msg,
    struct timespec *start,
    struct timespec *stop) {
  printf(msg, stop->tv_sec - start->tv_sec, stop->tv_nsec - start->tv_nsec);
}

int
main(int argc, char **argv) {
  struct timespec timestart, timestop;
  obj *array[4096];

  struct slab_cache cache;
  slab_cache_create(&cache, sizeof(obj), NULL, NULL);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 4096; i++) {
    array[i] = slab_cache_alloc(&cache);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Caching 4096 objects took: %d.%d seconds\n", &timestart, &timestop);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 4096; i++) {
    slab_cache_free(&cache, array[i]);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Freeing 4096 objects took: %d.%d seconds\n", &timestart, &timestop);

  return 0;
}
