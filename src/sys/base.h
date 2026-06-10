/*
  base.h
*/
#ifndef _BASE_H_
#define _BASE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define COUNTOF(a) (size)(sizeof(a) / sizeof(*(a)))
#define LENGTHOF(s) (COUNTOF(s) - 1)
#define NEW(type, numbytes) (type *)malloc(numbytes)
#define RESIZE(type, ptr, numbytes) (type *)realloc(ptr, numbytes)
#define DELETE(ptr)                                                            \
  {                                                                            \
    free(ptr);                                                                 \
    (ptr) = NULL;                                                              \
  }


/* --- convenient type aliases --- */
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int32_t b32;
typedef uintptr_t uptr;
typedef char byte;
typedef ptrdiff_t size;
typedef size_t usize;


/*
  Arena Allocator

  Provides a rather simple implementation of an arena allocator based off:
  https://www.youtube.com/watch?v=jgiMagdjA1

  It's a simpler way to manage memory in c by grouping related allocations
  in order to not manage them individually and potentially missing - or complicating -
  their deallocation.

  With an Arena you allocate a chunk of memory to use up front and then use that to
  allocate memory as you go. The arena is then destroyed when none of the objects
  that use it are no longer needed.

  This is a very simple arena implementation that uses malloc/free, but as I get more
  familiar with this technique and learn some more this may get replaced with a set
  of different allocators.

*/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- definitions --- */
#define MB 1000000

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

/*
  String8:
 *
 * Provides a more ergonomic implementation of Strings for C based off a few articles
 * - https://thasso.xyz/fixing-c-strings.html
 * - https://nullprogram.com/blog/2023/10/08/
 *
 * The idea is to wrap the c's null terminated string along with its length.
 *
 * String8 is meant to be created, read, and released (if needed), but not modified.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#define STRING8(s) (String8){ .data = s, .length = LENGTHOF(s) }

typedef struct String8 {
  char *data;
  u64 length;
} String8;

typedef struct String8Buffer {
  char *data;
  u64 length;
  u64 capacity;
} String8Buffer;

/* --- definitions ---*/

String8 string8_from_charbuf(MemoryArena *arena, char *buf, u64 length);
String8 string8_clone(MemoryArena *arena, String8 s);
String8 string8_concat(MemoryArena *arena, String8 lhs, String8 rhs);
String8 string8_join(MemoryArena *arena, String8 separator, usize count, String8 first, ...);
String8 string8_substringfrom(String8 s, u64 start_index);
char string8_get(String8 s, usize index);
size string8_compare(String8 lhs, String8 rhs);
bool string8_equals(String8 lhs, String8 rhs);
bool string8_startswith(String8 s, String8 prefix);
bool string8_endswith(String8 s, String8 suffix);

/*
   TODO: implement `String8 string8_trim(String8 s)`
   TODO: implement `String8 string8_upcase(String8 s)`
   TODO: implement `String8 string8_downcase(String8 s)`
   TODO: implement `String8 string8_replace(String8 s, String8 matching_text, String8 replacement)`
*/

/* --- implementation --- */

/*
 * Creates and returns an arena allocated String8 value
 * from `buf` with `length`.
 */
String8 string8_from_charbuf(MemoryArena *arena, char *buf, u64 length) {
  String8 new_str = {};
  new_str.data = (char*)arena_push(arena, length + 1);
  new_str.length = length;

  if (length) {
    memcpy(new_str.data, buf, length + 1);
  }
  return new_str;
}

/*
 * Returns a copy of `s` allocated in `arena`.
 */
String8 string8_clone(MemoryArena *arena, String8 s) {
  String8 new_str = {};
  new_str.data = (char *)arena_push(arena, s.length + 1);  // + 1 to store the null terminator \0
  new_str.length = s.length;

  if (s.length) {
    memcpy(new_str.data, s.data, s.length + 1);  // copying the null terminator
  }
  return new_str;
}

String8 string8_concat(MemoryArena *arena, String8 lhs, String8 rhs) {
  u64 new_length = lhs.length + rhs.length;

  String8 new_str = {};
  new_str.data = (char *)arena_push(arena, new_length + 1);  // + 1 to store the null terminator of rhs
  new_str.length = new_length;

  char *p = new_str.data;
  if (new_length) {
    memcpy(p, lhs.data, lhs.length);      // copy lhs string data
    p += lhs.length;                      // advance p to end of lhs data
    memcpy(p, rhs.data, rhs.length + 1);  // copy rhs string data along with the null terminator
  }
  return new_str;
}

// this may be better taking a collection of string8 values
String8 string8_join(MemoryArena *arena, String8 separator, usize count, String8 first, ...) {
  if (count == 0) { return STRING8(""); }
  if (count == 1) { return first; }

  // refer to:
  // - `man 3 stdarg`
  // - https://dev.to/pauljlucas/variadic-functions-in-c-53ml
  va_list values;
  va_start(values, first);

  String8 strings[count];
  strings[0] = first;

  // we know that the separator will be added between the strings being
  // joined, but won't be added after the last item. i.e. count - 1 times.
  u64 total_length = first.length + separator.length * (count - 1);
  for (usize i = 1; i < count; i++) {
    String8 s = va_arg(values, String8);
    total_length += s.length;
    strings[i] = s;
  }
  va_end(values);

  String8 new_str = { .length = total_length };
  new_str.data = (char *)arena_push(arena, total_length + 1); // + 1 to store the null terminator of the last string

  char *p = new_str.data;
  for (usize i = 0; i < count; i++) {
    memcpy(p, strings[i].data, strings[i].length);
    p += strings[i].length;

    // skip the separator on the last item
    if (i == (count - 1)) { continue; }

    // else add separator
    memcpy(p, separator.data, separator.length);
    p += separator.length;
  }
  *p = 0; // set the null terminator on the joined string
  return new_str;
}

char string8_get(String8 s, usize index) {
  if (index >= s.length) {
    return -1;
  }
  return s.data[index];
}

/*
 * Returns a substring view of `s` from `start_index` to its end.
 * The returned String8 is a slice/view, not an independent copy.
 */
String8 string8_substringfrom(String8 s, u64 start_index) {
  String8 slice;

  if (start_index >= s.length) {
    slice.length = 0;
    return slice;
  }

  slice.data = (s.data + start_index);
  slice.length = (s.length - start_index);
  return slice;
}

/*
 * Returns true if `lhs` and `rhs` contain the same characters.
 */
bool string8_equals(String8 lhs, String8 rhs) {
  if (lhs.length != rhs.length) {
    return false;
  }

  for (usize i = 0; i < lhs.length; i++) {
    if (string8_get(lhs, i) != string8_get(rhs, i)) {
      return false;
    }
  }

  return true;
}

/*
 * Compares two String8 values lexicographically.
 * Returns -1 if `lhs` is less than `rhs`.
 * Returns  1 if `lhs` is greater than `rhs`.
 * Returns  0 if `lhs` has the same char values as `rhs`.
 */
size string8_compare(String8 lhs, String8 rhs) {
  // select the smaller length to bind the iteration to
  usize s = lhs.length < rhs.length ? lhs.length : rhs.length;

  for (usize i = 0; i < s; i++) {
    // diff each char
    size diff = string8_get(lhs, i) - string8_get(rhs, i);

    if (diff != 0) {
      return diff;
    }
  }
  // if char-wise comparison is equal for all chars compare,
  // compare the length of the two String8
  return lhs.length - rhs.length;
}

/*
 * Returns true if `s` starts with `prefix`, false otherwise.
 * Returns true for an empty `prefix`.
 */
bool string8_startswith(String8 s, String8 prefix) {
  if (prefix.length > s.length) {
    return false;
  }

  if (prefix.length == 0) {
    return true;
  }

  for (usize i = 0; i < prefix.length; i++) {
    if (string8_get(s, i) != string8_get(prefix, i)) {
      return false;
    }
  }
  return true;
}

/*
 * Returns true if `s` ends with `suffix`, false otherwise.
 * Returns true for an empty `suffix`.
 */
bool string8_endswith(String8 s, String8 suffix) {
  if (suffix.length > s.length) {
    return false;
  }

  if (suffix.length == 0) {
    return true;
  }

  for (usize i = s.length - suffix.length; i < s.length; i++) {
    if (string8_get(s, i) != string8_get(suffix, i - suffix.length)) {
      return false;
    }
  }
  return true;
}

/* --- 2D conveniences --- */
typedef struct Point {
  i64 x;
  i64 y;
} Point;

typedef struct Rect {
  Point origin;
  Point corner;
} Rect;

i64 rect_width(Rect r);
i64 rect_height(Rect r);

i64 rect_width(Rect r) { return r.corner.x - r.origin.x; }
i64 rect_height(Rect r) { return r.corner.y - r.origin.y; }

#endif
