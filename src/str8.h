#ifndef _STR8_
#define _STR8_

#include <stdbool.h>
#include <string.h>

enum Str8Code {
  STR8ERR_OK,
  STR8ERR_INCOMPLETE_OP
};

typedef struct Str8 {
  bool allocated;
  u64 size;
  u8 *data;
} Str8;

#define str8lit(s)  (Str8){false, lengthof(s), (u8 *)s}  // use for literal strings (ie. "foo")
#define str8fmt(s8) (int)(s8).size, (s8).data

Str8 str8from_charbuff(char *buff, u64 size);
Str8 str8new(u64 size);
Str8 str8clone(Str8 s);
void str8free(Str8 *s);

Str8 str8from_charbuff(char *buff, u64 size) {
  Str8 s = str8new(size);
  memcpy(s.data, buff, size + 1);
  return s;
}

Str8 str8new(u64 size) {
  Str8 s = {};
  s.allocated = true;
  s.size = size;
  s.data = new(u8, size + 1);
  return s;
}

Str8 str8clone(Str8 s) {
  Str8 newstr = str8new(s.size);
  memcpy(newstr.data, s.data, s.size + 1);
  return newstr;
}

void str8free(Str8 *s) {
  if (s->allocated) {
    delete(s->data);
    s->size = 0;
  }
}

#endif
