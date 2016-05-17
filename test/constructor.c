#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "slab.h"

struct data_type {
  char val[4];
};

void constructor(void *data) {
  char *val = data;
  val[0] = 'a';
  val[1] = 'b';
  val[2] = 'c';
  val[3] = 'd';
}

void destructor(void *data) {
  char *val = data;
  val[0] = 'e';
  val[1] = 'f';
  val[2] = 'g';
  val[3] = 'h';
}


int
main(int argc, char **argv) {
  struct slab_cache cache;
  slab_cache_create(&cache, sizeof(struct data_type), constructor, destructor);
  char *obj;

  char* objs[100];
  for (int i = 0; i < 100; i++) {
    obj = slab_cache_alloc(&cache);
    objs[i] = obj;

    assert(obj[0] == 'a');
    assert(obj[1] == 'b');
    assert(obj[2] == 'c');
    assert(obj[3] == 'd');
  }

  int old_slab_count = cache.slab_count;
  for (int i = 0; i < 100; i++) {
    slab_cache_free(&cache, objs[i]);
    if (cache.slab_count < old_slab_count) {
      // We deleted the slab but we can't access the memory since the slab might have
      // been reclaimed, so there's no way to know if that has actually worked.
      old_slab_count = cache.slab_count;
    } else {
      assert(objs[i][0] == 'e');
      assert(objs[i][1] == 'f');
      assert(objs[i][2] == 'g');
      assert(objs[i][3] == 'h');
    }
  }

  slab_cache_destroy(&cache);
  assert(cache.slab_count == 0);

  printf("Everything ok.\n");

  return 0;
}
