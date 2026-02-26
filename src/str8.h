#include <stdbool.h>
#include <string.h>

enum Str8Code {
  STR8ERR_OK,
  STR8ERR_INCOMPLETE_OP
};

typedef struct Str8 {
  u64 size;
  u8 *data;
} Str8;

#define str8lit(s)  (Str8){lengthof(s), (u8 *)s}  // use for literal strings (ie. "foo")
#define str8fmt(s8) (int)(s8).size, (s8).data

Str8 *str8from_charbuff(char *buff, u64 size);
Str8 *str8new(u64 size);
Str8 *str8clone(Str8 *s);
bool str8cpy(Str8 *dst, Str8 *src);
void str8del(Str8 *s);

typedef struct Str8Array {
  usize capacity;
  usize count;
  Str8  **items;
} Str8Array;

Str8Array *str8array_create(void);
Str8Array *str8array_create_with_size(usize itemcount);
Str8Array *str8array_create_with(Str8 *items[], usize itemcount);
Str8Array *str8array_create_with_lit(char *lititems[], usize itemcount);

bool str8array_append(Str8Array *arr, Str8 *item);
bool str8array_append_lit(Str8Array *arr, char *lititem);

enum Str8Code str8array_append_all(Str8Array *arr, Str8 *items[], usize itemcount);
enum Str8Code str8array_append_lit_all(Str8Array *arr, char *lititem[], usize itemcount);

char* str8array_format(Str8Array *arr);
char* str8array_format_pretty(Str8Array *arr);

void str8array_del(Str8Array *arr);

Str8 *str8from_charbuff(char *buff, u64 size) {
  Str8 *s = str8new(size);
  memcpy(s->data, buff, size);
  return s;
}

Str8 *str8new(u64 size) {
  Str8 *s = new(Str8, sizeof(Str8));
  s->size = size;
  s->data = new(u8, size);
  return s;
}

Str8* str8clone(Str8 *s) {
  Str8 *newstr = str8new(s->size);
  if (!str8cpy(newstr, s)) {
    return NULL;
  }
  return newstr;
}

bool str8cpy(Str8 *dst, Str8 *src) {
  if (dst->size != src->size) {
    return false;
  }

  memcpy(dst->data, src->data, src->size);
  return true;
}

void str8del(Str8 *s) {
  delete(s->data);
  delete(s);
}

Str8Array *str8array_create(void) {
  Str8Array *arr = new(Str8Array, sizeof(Str8Array));
  if (arr == NULL) {
    return NULL;
  }

  arr->capacity = 0;
  arr->count    = 0;
  arr->items    = NULL;
  return arr;
}

Str8Array* str8array_create_with_size(usize itemcount) {
  Str8Array *arr = str8array_create();
  if (arr == NULL) {
    return NULL;
  }

  arr->capacity = itemcount;
  arr->count    = itemcount;
  arr->items    = resize(Str8*, arr->items, sizeof(Str8) * itemcount);
  return arr;
}

Str8Array *str8array_create_with(Str8 *items[], usize itemcount) {
  Str8Array *arr = str8array_create_with_size(itemcount);
  if (arr == NULL) {
    return NULL;
  }

  for (usize i = 0; i < itemcount; i++) {
    str8array_append(arr, items[i]);
  }

  return arr;
}

Str8Array *str8array_create_with_lit(char *lititems[], usize itemcount) {
  Str8Array *arr = str8array_create_with_size(itemcount);
  if (arr == NULL) {
    return NULL;
  }

  for (usize i = 0; i < itemcount; i++) {
    str8array_append_lit(arr, lititems[i]);
  }

  return arr;
}

bool str8array_append(Str8Array *arr, Str8 *item) {
  // resize array if we have reached its capacity
  if (arr->capacity < arr->count + 1) {
    usize newcapacity = (arr->capacity < 8) ? 8 : (arr->capacity * 2);

    Str8 **tmp = resize(Str8*, arr->items, (sizeof(Str8) * newcapacity));
    if (tmp == NULL) {
      return false;
    }

    arr->capacity = newcapacity;
    arr->items = tmp;
  }

  // allocate memory for the new Str8 item
  arr->items[arr->count] = str8new(item->size);
  if (arr->items[arr->count] == NULL) {
    return false;
  }

  bool success = str8cpy(arr->items[arr->count], item);
  if (!success) {
    return false;
  }

  // we successfully added the new item
  arr->count++;
  return true;
}

bool str8array_append_lit(Str8Array *arr, char *lititem) {
  u64 size = (u64)strlen(lititem);
  return str8array_append(arr, str8from_charbuff(lititem, size));
}

enum Str8Code str8array_append_all(Str8Array *arr, Str8 *items[], usize itemcount) {
  for (usize i = 0; i < itemcount; i++) {
    if (!str8array_append(arr, items[i])) {
      return STR8ERR_INCOMPLETE_OP;
    }
  }
  return STR8ERR_OK;
}

enum Str8Code str8array_append_lit_all(Str8Array *arr, char *lititems[], usize itemcount) {
  for (usize i = 0; i < itemcount; i++) {
    if (!str8array_append_lit(arr, lititems[i])) {
      return STR8ERR_INCOMPLETE_OP;
    }
  }
  return STR8ERR_OK;
}

char* str8array_format(Str8Array *arr) {
  return "";
}

char* str8array_format_pretty(Str8Array *arr) {
  return "";
}

void str8array_destroy(Str8Array *arr) {
  for (usize i = 0; i < arr->count; i++) {
    str8del(arr->items[i]);
  }
  delete(arr);
}
