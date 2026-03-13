/*
 * string8.h
 *
 * Provides a more ergonomic implementation of Strings for C based off a few articles
 * - https://thasso.xyz/fixing-c-strings.html
 * - https://nullprogram.com/blog/2023/10/08/
 *
 * The idea is to wrap the c's null terminated string along with its length.
 *
 * String8 is meant to be created, read, and released (if needed), but not modified.
 */

#ifndef __STRING8_H__
#define __STRING8_H__

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

// ******************************
// |        DEFINITIONS         |
// ******************************

String8 string8_from_charbuf(char *buf, u64 length, MemoryArena *arena);
String8 string8_clone(String8 s, MemoryArena *arena);
String8 string8_substringfrom(String8 s, u64 start_index);
char string8_get(String8 s, usize index);
size string8_compare(String8 lhs, String8 rhs);
bool string8_equals(String8 lhs, String8 rhs);
bool string8_startswith(String8 s, String8 prefix);
bool string8_endswith(String8 s, String8 suffix);

/* TODO implement
String8 String8_trim(String8 s);
String8 String8_upcase(String8 s);
String8 String8_downcase(String8 s);
*/


// ******************************
// |      IMPLEMENTATIONS       |
// ******************************

/*
 * Creates and returns an arena allocated String8 value
 * from `buf` with `length`.
 */
String8 string8_from_charbuf(char *buf, u64 length, MemoryArena *arena) {
  String8 new_str = {};
  new_str.data = (char*)arena_push(arena, length, false);
  new_str.length = length;

  if (length) {
    memcpy(new_str.data, buf, length);
  }
  return new_str;
}

/*
 * Returns a copy of `s` allocated in `arena`.
 */
String8 string8_clone(String8 s, MemoryArena *arena) {
  String8 new_str = {};
  new_str.data = (char *)arena_push(arena, s.length, false);
  new_str.length = s.length;

  if (s.length) {
    memcpy(new_str.data, s.data, s.length);
  }
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

#endif
