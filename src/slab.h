#ifndef _SLAB_H
#define _SLAB_H

#include <stdbool.h>
#include <unistd.h>
#include "queue.h"

#define SLAB_FILL_CONSTRUCT 0xAABBCCDD

struct slab {
  unsigned capacity;
  unsigned used;
  SLIST_ENTRY(slab) entries;

  // Keep the buffer as the last item because we allocate a set amount of memory for the whole
  // struct but the compiler is not aware of that, therefore if attempt to access an item at
  // buf+4 we won't overflow since the memory is correctly allocated, however if buf was not
  // the last item, changing the item at buf+8 would change whatever field follows in the
  // struct.
  void *buf;
};

struct slab_cache {
  ssize_t obj_size;
  void (*constructor)(void *);
  void (*destructor)(void *);

  unsigned int slab_count;
  unsigned int __max_items_per_slab;
  SLIST_HEAD(, slab) __slabs;
};

int
slab_cache_create(struct slab_cache *,
                  size_t,
                  void (*constructor)(void *),
                  void (*destructor)(void *));

void
slab_cache_destroy(struct slab_cache *);


void *
slab_cache_alloc(struct slab_cache *);


void
slab_cache_free(struct slab_cache *, void *);


#endif
