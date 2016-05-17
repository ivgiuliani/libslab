#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#ifdef CASSERT
#  include <assert.h>
#  define ASSERT(x) assert((x))
#else
#  define ASSERT(x)
#endif

// We ship our own version of queue.h because the one bundled with
// mainstream linux distributions is an ancient version that does not
// provide the _SAFE variations of the list methods.
#include "queue.h"

#include "slab.h"

#define __slab_is_full(slab) (slab->used == slab->capacity)
#define __slab_is_empty(slab) (slab->used == 0)

static const size_t __get_slab_size() {
  static ssize_t page_size = -1;
  if (page_size == -1) {
    page_size = getpagesize();
  }
  ASSERT(page_size != 0);
  return page_size;
}


struct slab *
__slab_alloc(struct slab_cache *cache) {
  ASSERT(cache->obj_size <= __get_slab_size());

  // mmap is necessary to get the memory aligned to page boundaries.
  struct slab *slab = (struct slab *) mmap(
      NULL,
      __get_slab_size(),
      PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED,
      -1, 0
  );

  slab->capacity = cache->__max_items_per_slab;
  slab->used = 0;

  for (int i = 0; cache->constructor != NULL && i < slab->capacity; i++) {
    void *obj_in_slab = (void *)&slab->buf + (cache->obj_size * i);
    (*cache->constructor)(obj_in_slab);
  }

  return slab;
}


void
__slab_free(struct slab *slab) {
  munmap(slab, __get_slab_size());
}

/**
 * Finds a slab with at least a free entry or NULL if all the
 * slabs are full.
 */
struct slab *
__slab_pick(struct slab_cache *cache) {
  // Pick the fullest (yet not completely full) of the slabs so that
  // we keep memory fragmentation to a minimum.
  struct slab *slab = NULL;
  struct slab *best_slab = NULL;
  SLIST_FOREACH(slab, &cache->__slabs, entries) {
    if (!__slab_is_full(slab) &&
        (best_slab == NULL || slab->used > best_slab->used)) {
      best_slab = slab;

      if (best_slab->used == best_slab->capacity - 1) {
        // Can't get any better than this.
        break;
      }
    }
  }

  return best_slab;
}


struct slab *
__slab_grow(struct slab_cache *cache) {
  struct slab *slab = __slab_alloc(cache);
  cache->slab_count++;
  SLIST_INSERT_HEAD(&cache->__slabs, slab, entries);
  return slab;
}


int
slab_cache_create(struct slab_cache *cache,
                  size_t obj_size,
                  void (*constructor)(void *),
                  void (*destructor)(void *)) {
  if (obj_size > __get_slab_size() / 2) {
    // Cannot allocate objects this big.
    return -1;
  }

  cache->obj_size = obj_size;
  cache->constructor = constructor;
  cache->destructor = destructor;
  cache->slab_count = 0;
  cache->__max_items_per_slab = (__get_slab_size() - sizeof(struct slab)) / cache->obj_size;
  SLIST_INIT(&cache->__slabs);

  return 0;
}


void
slab_cache_destroy(struct slab_cache *cache) {
  struct slab *slab, *slab_tmp;
  SLIST_FOREACH_SAFE(slab, &cache->__slabs, entries, slab_tmp) {
    SLIST_REMOVE(&cache->__slabs, slab, slab, entries);
    __slab_free(slab);
    cache->slab_count--;
  }
  cache->obj_size = 0;
}


void *
slab_cache_alloc(struct slab_cache *cache) {
  struct slab *slab = __slab_pick(cache);
  if (slab == NULL) {
    slab = __slab_grow(cache);
  }

  if (slab == NULL) {
    return NULL;
  }

  void *obj_in_slab = (void *)&slab->buf + (cache->obj_size * slab->used);
  slab->used++;

  return obj_in_slab;
}


void
slab_cache_free(struct slab_cache *cache, void *obj) {
  const ssize_t slab_addr_space = __get_slab_size() - sizeof(struct slab);

  struct slab *slab, *slab_tmp;
  void *slab_start_addr, *slab_end_addr;

  SLIST_FOREACH_SAFE(slab, &cache->__slabs, entries, slab_tmp) {
    slab_start_addr = &slab->buf;
    slab_end_addr = slab_start_addr + slab_addr_space;

    const bool is_obj_in_slab = obj >= slab_start_addr && obj < slab_end_addr;
    if (is_obj_in_slab) {
      slab->used--;

      if (__slab_is_empty(slab)) {
        SLIST_REMOVE(&cache->__slabs, slab, slab, entries);
        cache->slab_count--;

        void *obj_in_slab;
        for (int i = 0; cache->destructor != NULL && i < slab->capacity; i++) {
          obj_in_slab = (void *)&slab->buf + (cache->obj_size * i);
          (*cache->destructor)(obj);
        }

        __slab_free(slab);
      }

      // We found the object and performed any necessary cleanup. No need to further
      // iterate on the remaining slabs.
      return;
    }
  }
}
