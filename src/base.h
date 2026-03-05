#ifndef _BASE_H_
#define _BASE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define COUNTOF(a) (size)(sizeof(a) / sizeof(*(a)))
#define LENGTHOF(s) (COUNTOF(s) - 1)
#define NEW(type, numbytes) (type *)malloc(numbytes)
#define RESIZE(type, ptr, numbytes) (type *)realloc(ptr, numbytes)
#define DELETE(ptr)                                                            \
  {                                                                            \
    free(ptr);                                                                 \
    (ptr) = NULL;                                                              \
  }

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

#endif
