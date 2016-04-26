#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

// We have to ship our own version of queue.h because the one bundled with
// Ubuntu is an ancient version that does not provide the _SAFE variations
// of the list methods.
#include "queue.h"

#include "slab.h"

static const size_t __get_slab_size() {
  static ssize_t page_size = 0;
  if (page_size == 0) {
    page_size = getpagesize();
  }
  return page_size;
}


struct slab *
__slab_alloc(struct slab_cache *cache) {
  assert(cache->obj_size <= __get_slab_size());

  struct slab *slab = (struct slab *) mmap(
      NULL,
      __get_slab_size(),
      PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED,
      -1, 0
  );
  memset(slab, 0, __get_slab_size());

  ssize_t available_space = __get_slab_size() - sizeof(struct slab);
  unsigned max_items = available_space / cache->obj_size;
  slab->capacity = max_items;
  slab->used = 0;

  return slab;
}


void
__slab_free(struct slab *slab) {
  munmap(slab, __get_slab_size());
}


__force_inline bool
__slab_is_full(struct slab *slab) {
  return slab->used == slab->capacity;
}


__force_inline bool
__slab_is_empty(struct slab *slab) {
  return slab->used == 0;
}


/**
 * Finds a slab with at least a free entry or NULL if all the
 * slabs are full.
 */
struct slab *
__slab_pick(struct slab_cache *cache) {
  struct slab *slab = NULL;
  STAILQ_FOREACH(slab, &cache->__slabs, entries) {
    if (!__slab_is_full(slab)) {
      break;
    }
  }
  return slab;
}


struct slab *
__slab_grow(struct slab_cache *cache) {
  struct slab *slab = __slab_alloc(cache);
  cache->slab_count++;
  STAILQ_INSERT_TAIL(&cache->__slabs, slab, entries);
  return slab;
}


void
slab_cache_create(struct slab_cache *cache,
                  size_t obj_size,
                  void (*constructor)(void *),
                  void (*destructor)(void *)) {
  cache->obj_size = obj_size;
  cache->constructor = constructor;
  cache->destructor = destructor;
  cache->slab_count = 0;
  STAILQ_INIT(&cache->__slabs);
}

void
slab_cache_destroy(struct slab_cache *cache) {
  struct slab *slab, *slab_tmp;
  STAILQ_FOREACH_SAFE(slab, &cache->__slabs, entries, slab_tmp) {
    STAILQ_REMOVE(&cache->__slabs, slab, slab, entries);
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
  if (cache->constructor != NULL) {
    (*cache->constructor)(obj_in_slab);
  }

  slab->used++;

  return obj_in_slab;
}

void
slab_cache_free(struct slab_cache *cache, void *obj) {
  struct slab *slab, *slab_tmp;
  void *slab_start_addr, *slab_end_addr;

  STAILQ_FOREACH_SAFE(slab, &cache->__slabs, entries, slab_tmp) {
    slab_start_addr = &slab->buf;
    slab_end_addr = slab_start_addr + __get_slab_size() - sizeof(struct slab);

    const bool is_obj_in_slab = obj >= slab_start_addr && obj < slab_end_addr;
    if (is_obj_in_slab) {
      slab->used--;

      if (cache->destructor != NULL) {
        (*cache->destructor)(obj);
      }

      if (__slab_is_empty(slab)) {
        STAILQ_REMOVE(&cache->__slabs, slab, slab, entries);
        cache->slab_count--;
        __slab_free(slab);
      }
    }
  }
}
