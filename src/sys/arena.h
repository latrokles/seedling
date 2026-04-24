/*
* arena.h
*
* Provides a rather simple implementation of an arena allocator based off:
* https://www.youtube.com/watch?v=jgiMagdjA1
*
* It's a simpler way to manage memory in c by grouping related allocations
* in order to not manage them individually and potentially missing - or complicating -
* their deallocation.
*
* With an Arena you allocate a chunk of memory to use up front and then use that to
* allocate memory as you go. The arena is then destroyed when none of the objects
* that use it are no longer needed.
*
* This is a very simple arena implementation that uses malloc/free, but as I get more
* familiar with this technique and learn some more this may get replaced with a set
* of different allocators.
*/

#ifndef __MEM_ARENA_H__
#define __MEM_ARENA_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"

/* --- definitions --- */

typedef struct MemoryArena {
  u64 capacity;
  u64 position;
  u8  *memory;
} MemoryArena;

MemoryArena *arena_create(u64 capacity);
void arena_destroy(MemoryArena *arena);
void *arena_push(MemoryArena *arena, u64 size);
void *arena_push_nozero(MemoryArena *arena, u64 size);
void arena_pop(MemoryArena *arena, u64 size);
void arena_pop_to(MemoryArena *arena, u64 pos);
void arena_clear(MemoryArena *arena);

/* --- implementation --- */

MemoryArena *arena_create(u64 capacity) {
  MemoryArena *arena = malloc(sizeof(MemoryArena));
  assert(arena != NULL);

  arena->capacity = capacity;
  arena->position = 0;
  arena->memory = malloc((sizeof(u8)) * capacity);
  assert(arena->memory != NULL);

  memset(arena->memory, 0, capacity);
  return arena;
}

void arena_destroy(MemoryArena *arena) {
  DELETE(arena->memory);
  DELETE(arena);
}

void *arena_push(MemoryArena *arena, u64 size) {
  void *data = arena_push_nozero(arena, size);
  memset(data, 0, size);
  return data;
}

void *arena_push_nozero(MemoryArena *arena, u64 size) {
  assert(arena->position + size < arena->capacity);

  u8 *data = &arena->memory[arena->position];
  arena->position += size;

  return data;
}

void arena_pop(MemoryArena *arena, u64 size) {
  assert(size <= arena->position);
  arena->position -= size;
}

void arena_pop_to(MemoryArena *arena, u64 pos) {
  u64 size = pos <= arena->position ? arena->position - pos : 0;
  arena_pop(arena, size);
}

void arena_clear(MemoryArena *arena) {
  arena_pop_to(arena, 0);
}

#ifdef DEBUG_MEMORY

/* --- debugging ---
   if DEBUG_MEMORY is enabled (i.e. -DDEBUG_MEMORY), swap memory functions with debug versions
   that print debugging information to STDERR.
*/

MemoryArena *debug_arena_create(u64 capacity, char *filename, u64 linenumber);
void debug_arena_destroy(MemoryArena *arena, char *filename, u64 linenumber);
void *debug_arena_push(MemoryArena *arena, u64 size, char *filename, u64 linenumber);
void *debug_arena_push_nozero(MemoryArena *arena, u64 size, char *filename, u64 linenumber);
void debug_arena_pop(MemoryArena *arena, u64 size, char *filename, u64 linenumber);
void debug_arena_pop_to(MemoryArena *arena, u64 pos, char *filename, u64 linenumber);
void debug_arena_clear(MemoryArena *arena, char *filename, u64 linenumber);

MemoryArena *debug_arena_create(u64 capacity, char *filename, u64 linenumber) {
  MemoryArena *arena = arena_create(capacity);
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_create(capacity=%zu), ", filename, linenumber, capacity);
  fprintf(stderr, "created MemoryArena(capacity=%zu, position=%zu).\n", arena->capacity, arena->position);
  return arena;
}

void debug_arena_destroy(MemoryArena *arena, char *filename, u64 linenumber) {
  arena_destroy(arena);
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_destroy, ", filename, linenumber);
  fprintf(stderr, "destroyed MemoryArena(capacity=%zu, position=%zu).\n", arena->capacity, arena->position);
}

void *debug_arena_push(MemoryArena *arena, u64 size, char *filename, u64 linenumber) {
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_push(..., size=%zu) ", filename, linenumber, size);
  fprintf(stderr, "BEFORE=MemoryArena(capacity=%zu, position=%zu), ", arena->capacity, arena->position);
  void *data = arena_push(arena, size);
  fprintf(stderr, "AFTER=MemoryArena(capacity=%zu, position=%zu), data_ptr=%p.\n", arena->capacity, arena->position, &data);
  return data;
}

void *debug_arena_push_nozero(MemoryArena *arena, u64 size, char *filename, u64 linenumber) {
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_push_nozero(..., size=%zu) ", filename, linenumber, size);
  fprintf(stderr, "BEFORE=MemoryArena(capacity=%zu, position=%zu), ", arena->capacity, arena->position);
  void *data = arena_push(arena, size);
  fprintf(stderr, "AFTER=MemoryArena(capacity=%zu, position=%zu), data_ptr=%p.\n", arena->capacity, arena->position, &data);
  return data;
}

void debug_arena_pop(MemoryArena *arena, u64 size, char *filename, u64 linenumber) {
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_pop(..., size=%zu) ", filename, linenumber, size);
  fprintf(stderr, "BEFORE=MemoryArena(capacity=%zu, position=%zu), ", arena->capacity, arena->position);
  arena_pop(arena, size);
  fprintf(stderr, "AFTER=MemoryArena(capacity=%zu, position=%zu).\n", arena->capacity, arena->position);
}

void debug_arena_pop_to(MemoryArena *arena, u64 pos, char *filename, u64 linenumber) {
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_pop_to(..., pos=%zu) ", filename, linenumber, pos);
  fprintf(stderr, "BEFORE=MemoryArena(capacity=%zu, position=%zu), ", arena->capacity, arena->position);
  arena_pop_to(arena, pos);
  fprintf(stderr, "AFTER=MemoryArena(capacity=%zu, position=%zu).\n", arena->capacity, arena->position);
}

void debug_arena_clear(MemoryArena *arena, char *filename, u64 linenumber) {
  fprintf(stderr, "DEBUG_MEM[%s, %zu]: arena_clear(...) ", filename, linenumber);
  fprintf(stderr, "BEFORE=MemoryArena(capacity=%zu, position=%zu), ", arena->capacity, arena->position);
  arena_clear(arena);
  fprintf(stderr, "AFTER=MemoryArena(capacity=%zu, position=%zu).\n", arena->capacity, arena->position);
}

#define arena_create(capacity)          debug_arena_create(capacity, __FILE__, __LINE__)
#define arena_destroy(arena)            debug_arena_destroy(arena, __FILE__, __LINE__)
#define arena_push(arena, size)         debug_arena_push(arena, size, __FILE__, __LINE__)
#define arena_push_nozero(arena, size)  debug_arena_push_nozero(arena, size, __FILE__, __LINE__)
#define arena_pop(arena, size)          debug_arena_pop(arena, size, __FILE__, __LINE__)
#define arena_pop_to(arena, pos)        debug_arena_pop_to(arena, pos, __FILE__, __LINE__)
#define arena_clear(arena)              debug_arena_clear(arena, __FILE__, __LINE__)

#endif


#endif
