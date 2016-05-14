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

typedef char onebyte[1];

void
measure(const char *msg,
    struct timespec *start,
    struct timespec *stop) {
  printf(msg, stop->tv_sec - start->tv_sec, stop->tv_nsec - start->tv_nsec);
}

int
main(int argc, char **argv) {
  struct timespec timestart, timestop;
  struct slab_cache cache;

  // Generic objects.
  obj *array[4096];

  slab_cache_create(&cache, sizeof(obj), NULL, NULL);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 4096; i++) {
    array[i] = slab_cache_alloc(&cache);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Caching 4096 generic objects took: %d.%d seconds\n", &timestart, &timestop);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 4096; i++) {
    slab_cache_free(&cache, array[i]);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Freeing 4096 generic objects took: %d.%d seconds\n", &timestart, &timestop);

  slab_cache_destroy(&cache);

  // 1 byte objects.
  onebyte *ob[10000];
  slab_cache_create(&cache, sizeof(onebyte), NULL, NULL);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 10000; i++) {
    ob[i] = slab_cache_alloc(&cache);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Caching 10000 1-byte objects took: %d.%d seconds\n", &timestart, &timestop);

  clock_gettime(CLOCK_REALTIME, &timestart);
  for (int i = 0; i < 10000; i++) {
    slab_cache_free(&cache, ob[i]);
  }
  clock_gettime(CLOCK_REALTIME, &timestop);
  measure("Freeing 10000 1-byte objects took: %d.%d seconds\n", &timestart, &timestop);

  slab_cache_destroy(&cache);

  return 0;
}
