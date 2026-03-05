#ifndef _MEM_ARENA_H_
#define _MEM_ARENA_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_BASE_POS      (sizeof(MemoryArena))
#define ARENA_ALIGN         (sizeof(uptr))

#define min(a, b)           (((a) < (b)) ? (a) : (b))
#define align_up_pow2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

typedef struct MemoryArena {
  u64 capacity;
  u64 position;
} MemoryArena;

// ******************************
// |        definitions         |
// ******************************

MemoryArena *arena_create(u64 capacity);
void         arena_destroy(MemoryArena *arena);
void *arena_push(MemoryArena *arena, u64 size, bool nonzero);
void  arena_pop(MemoryArena *arena, u64 size);
void  arena_pop_to(MemoryArena *arena, u64 pos);
void  arena_clear(MemoryArena *arena);

// ******************************
// |      implementations       |
// ******************************


MemoryArena *arena_create(u64 capacity) {
  MemoryArena *arena = (MemoryArena*)malloc(capacity);
  arena->capacity = capacity;
  arena->position = ARENA_BASE_POS;
  return arena;
}

void         arena_destroy(MemoryArena *arena) {
  free(arena);
}

void *arena_push(MemoryArena *arena, u64 size, bool nonzero) {
  u64 pos_aligned = align_up_pow2(arena->position, ARENA_ALIGN);
  u64 new_pos = pos_aligned + size;

  if (new_pos > arena->capacity) {
    fprintf(stderr, "ERROR: arena does not have enough capacity");
    exit(1);
  }

  arena->position = new_pos;
  u8* out = (u8*)arena + pos_aligned;

  if (!nonzero) {
    memset(out, 0, size);
  }

  return out;
}
void  arena_pop(MemoryArena *arena, u64 size) {
  size = min(size, arena->position - ARENA_BASE_POS);
  arena->position -= size;
}

void  arena_pop_to(MemoryArena *arena, u64 pos) {
  u64 size = pos < arena->position ? arena->position - pos : 0;
  arena_pop(arena, size);
}

void  arena_clear(MemoryArena *arena) {
  arena_pop_to(arena, ARENA_BASE_POS);
}

#endif
