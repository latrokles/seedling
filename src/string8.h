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

// ******************************
// |        DEFINITIONS         |
// ******************************

String8 string8_from_charbuf(char *buf, u64 length, MemoryArena *arena);
String8 string8_clone(String8 s, MemoryArena *arena);
String8 string8_concat(String8 lhs, String8 rhs, MemoryArena *arena);
String8 string8_join(MemoryArena *arena, String8 separator, usize count, String8 first, ...);
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

String8 string8_concat(String8 lhs, String8 rhs, MemoryArena *arena) {
  u64 new_length = lhs.length + rhs.length;

  String8 new_str = {};
  new_str.data = (char *)arena_push(arena, new_length, false);
  new_str.length = new_length;

  char *p = new_str.data;
  if (new_length) {
    memcpy(p, lhs.data, lhs.length);  // copy lhs string data
    p += lhs.length;                  // advance p to end of lhs data
    memcpy(p, rhs.data, rhs.length);  // copy rhs string data
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
  new_str.data = (char *)arena_push(arena, total_length, false);

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
