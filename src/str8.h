#ifndef _STR8_
#define _STR8_

#include <stdbool.h>
#include <string.h>

/* Str8 are meant to be read, not written to */
typedef struct Str8 {
  u64 size;
  char *data;
} Str8;

/* TODO define Str8Buff to be written to and read from */

#define str8lit(s)  (Str8){lengthof(s), (char *)s}  // use for literal strings (ie. "foo")
#define str8fmt(s8) (int)(s8).size, (s8).data

/*
 * TODO implement more functionality
 * str8cmp
 * str8trim
 * str8strip
 * str8rstrip
 * str8lstrip
 * str8upcase
 * str8downcase
 */

Str8 str8from_charbuff(char *buff, u64 size);
Str8 str8clone(MemoryArena *arena, Str8 s);
bool str8equals(Str8 lhs, Str8 rhs);


Str8 str8from_charbuff(char *buff, u64 size) {
  Str8 s = {};
  s.data = buff;
  s.size = size;
  return s;
}

Str8 str8clone(MemoryArena *arena, Str8 s) {
  Str8 new_str = {};
  new_str.data = (char *)arena_push(arena, s.size, false);
  new_str.size = s.size;

  if (s.size) {
    memcpy(new_str.data, s.data, s.size);
  }
  return new_str;
}

#endif
