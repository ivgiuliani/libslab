#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "slab.h"

#define ITEM_COUNT 12345

typedef struct __obj {
  int value;
} obj;

int
main(int argc, char **argv) {
  int slab_count = 0;
  struct slab *slab;

  struct slab_cache cache;
  slab_cache_create(&cache, sizeof(obj), NULL, NULL);
  assert(cache.slab_count == 0);

  obj* obj_array[ITEM_COUNT];
  obj *item;
  for (int i = 0; i < ITEM_COUNT; i++) {
    item = slab_cache_alloc(&cache);
    item->value = i;
    obj_array[i] = item;
  }

  int total_obj_count = 0;
  SLIST_FOREACH(slab, &cache.__slabs, entries) {
    total_obj_count += slab->used;
  }
  assert(total_obj_count == ITEM_COUNT);
  assert(cache.slab_count >= 1);

  for (int i = 0; i < ITEM_COUNT; i++) {
    assert(obj_array[i] != NULL);
    assert(obj_array[i]->value == i);
    slab_cache_free(&cache, obj_array[i]);
  }

  SLIST_FOREACH(slab, &cache.__slabs, entries) {
    slab_count++;
  }
  assert(slab_count == 0);

  slab_cache_destroy(&cache);
  assert(slab_count == 0);

  printf("Everything ok.\n");

  return 0;
}
