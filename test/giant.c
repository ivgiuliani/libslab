#include <assert.h>
#include <stdio.h>

#include "queue.h"
#include "slab.h"

int
main(int argc, char **argv) {
  struct slab_cache cache;
  assert(slab_cache_create(&cache, 1024 * 1024, NULL, NULL) == -1);

  printf("Everything ok.\n");

  return 0;
}
