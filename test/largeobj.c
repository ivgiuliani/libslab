#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "slab.h"

struct large_obj {
  char padding[1024];
};


int
main(int argc, char **argv) {
  struct slab_cache cache;
  slab_cache_create(&cache, sizeof(struct large_obj), NULL, NULL);
  int *obj;

  int* objs[100];
  for (int i = 0; i < 100; i++) {
    obj = slab_cache_alloc(&cache);
    *obj = i;
    objs[i] = obj;
  }
  assert(cache.slab_count == 34);

  for (int i = 0; i < 100; i++) {
    slab_cache_free(&cache, objs[i]);
  }

  slab_cache_destroy(&cache);
  assert(cache.slab_count == 0);

  int slab_count = 0;
  struct slab *slab;
  SLIST_FOREACH(slab, &cache.__slabs, entries) {
    slab_count++;
  }
  assert(slab_count == 0);

  printf("Everything ok.\n");

  return 0;
}
