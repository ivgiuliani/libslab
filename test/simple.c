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
  printf("slab_cache_create() ok.\n");

  assert(cache.obj_size == sizeof(obj));

  obj *o1;
  obj *o2;
  obj *o3;

  o1 = slab_cache_alloc(&cache);
  assert(o1 != NULL);
  printf("o1 = slab_cache_alloc() ok.\n");
  o2 = slab_cache_alloc(&cache);
  assert(o2 != NULL);
  printf("o2 = slab_cache_create() ok.\n");
  o3 = slab_cache_alloc(&cache);
  assert(o3 != NULL);
  printf("o3 = slab_cache_create() ok.\n");

  o1->value = 1;
  printf("o1->value = 1 ok.\n");
  o2->value = 2;
  printf("o2->value = 2 ok.\n");
  o3->value = 3;
  printf("o3->value = 3 ok.\n");

  slab_cache_free(&cache, o3);
  printf("slab_cache_free(o3) ok.\n");
  slab_cache_free(&cache, o2);
  printf("slab_cache_free(o2) ok.\n");
  slab_cache_free(&cache, o1);
  printf("slab_cache_free(o1) ok.\n");

  slab_cache_destroy(&cache);
  printf("slab_cache_destroy ok.\n");

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
