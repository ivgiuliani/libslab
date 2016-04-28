#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "slab.h"

typedef struct __obj {
  int value;
} obj;

int
main(int argc, char **argv) {
  struct slab_cache cache;
  slab_cache_create(&cache, sizeof(obj), NULL, NULL);

  assert(cache.obj_size == sizeof(obj));

  obj *o1;
  obj *o2;
  obj *o3;

  o1 = slab_cache_alloc(&cache);
  assert(o1 != NULL);
  o2 = slab_cache_alloc(&cache);
  assert(o2 != NULL);
  o3 = slab_cache_alloc(&cache);
  assert(o3 != NULL);

  o1->value = 1;
  o2->value = 2;
  o3->value = 3;

  slab_cache_free(&cache, o3);
  slab_cache_free(&cache, o2);
  slab_cache_free(&cache, o1);

  slab_cache_destroy(&cache);

  assert(cache.slab_count == 0);

  int slab_count = 0;
  struct slab *slab;
  STAILQ_FOREACH(slab, &cache.__slabs, entries) {
    slab_count++;
  }
  assert(slab_count == 0);

  printf("Everything ok.\n");

  return 0;
}
