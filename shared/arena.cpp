#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

// based on https://github.com/thejefflarson/arena

#define PAGE_SIZE 4095

typedef struct arena {
  uint8_t *region;
  size_t size;
  size_t current;
  struct arena *next;
} MemoryArena;

static MemoryArena *
_arena_create(size_t size) {
  MemoryArena *arena = (MemoryArena *) calloc(1, sizeof(MemoryArena));
  if(!arena) return NULL;
  arena->region = (uint8_t *) calloc(size, sizeof(uint8_t));
  arena->size   = size;
  if(!arena->region) { free(arena); return NULL; }
  return arena;
}

MemoryArena *
arena_create() {
  return _arena_create(PAGE_SIZE);
}

void *
arena_malloc(MemoryArena *arena, size_t size) {
  MemoryArena *last = arena;

  do {
    if((arena->size - arena->current) >= size){
      arena->current += size;
      return arena->region + (arena->current - size);
    }
    last = arena;
  } while((arena = arena->next) != NULL);

  size_t asize   = size > PAGE_SIZE ? size : PAGE_SIZE;
  MemoryArena *next  = _arena_create(asize);
  last->next     = next;
  next->current += size;
  return next->region;
}

void
arena_destroy(MemoryArena *arena) {
  MemoryArena *next, *last = arena;
  do {
    next = last->next;
    free(last->region);
    free(last);
    last = next;
  } while(next != NULL);
}
