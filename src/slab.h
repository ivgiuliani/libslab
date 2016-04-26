#ifndef _SLAB_H
#define _SLAB_H

#include <stdbool.h>
#include <unistd.h>
#include "queue.h"

#if __GNUC__
#  define __force_inline __attribute__((always_inline)) inline
#else
#  define __force_inline inline
#endif


struct slab {
  unsigned capacity;
  unsigned used;
  void *buf;

  STAILQ_ENTRY(slab) entries;
};

struct slab_cache {
  ssize_t obj_size;
  void (*constructor)(void *);
  void (*destructor)(void *);

  unsigned int slab_count;
  STAILQ_HEAD(, slab) __slabs;
};

void
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
