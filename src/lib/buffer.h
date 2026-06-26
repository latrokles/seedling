#ifndef _BUFFER_
#define _BUFFER_

/*
  buffer.h - a simple implementation of a gap buffer for text editing.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"

#define GAP_SIZE_BYTES 512

typedef struct GapBuffer {
  char *buf;
  i32   gap_start; // start of the gap
  i32   gap_end;   // end of the gap
  i32   size;      // buffer + size tells us the end of the buffer
} GapBuffer;


GapBuffer buffer_create();
void      buffer_destroy(GapBuffer *gb);
void      buffer_insert(GapBuffer *gb, char c);
void      buffer_backspace(GapBuffer *gb);
void      buffer_print(GapBuffer *gb);

i32  __buffer_gap_size(GapBuffer *gb);
void __buffer_grow(GapBuffer *gb);

GapBuffer buffer_create() {
  char *buf = malloc(GAP_SIZE_BYTES);

  // create the buffer with the entire buffer being the gap
  return (GapBuffer){
    .buf       = buf,
    .gap_start = 0,
    .gap_end   = GAP_SIZE_BYTES,
    .size      = GAP_SIZE_BYTES,
  };
}

void      buffer_destroy(GapBuffer *gb) {
  free(gb->buf);
  gb->gap_start = 0;
  gb->gap_end = 0;
  gb->size = 0;
}

void      buffer_insert(GapBuffer *gb, char c) {
  if (__buffer_gap_size(gb) == 0) {
    __buffer_grow(gb);
  }

  gb->buf[gb->gap_start++] = c;
}

void      buffer_backspace(GapBuffer *gb) {
  if (gb->gap_start == 0) {
    return;
  }
  gb->gap_start--;
}

i32  __buffer_gap_size(GapBuffer *gb) {
  return gb->gap_end - gb->gap_start;
}

void buffer_print(GapBuffer *gb) {
  // print data to the left of the gap
  for (i32 li=0; li < gb->gap_start; li++) {
    printf("%c", gb->buf[li]);
  }

  // print the gap
  for (i32 gi = gb->gap_start; gi < gb->gap_end; gi++) {
    printf(" _ ");
  }

  // print data to the right of the gap
  for (i32 ri=gb->gap_end; ri < gb->size; ri++) {
    printf("%c", gb->buf[ri]);
  }
  printf("\n");
}

void __buffer_grow(GapBuffer *gb) {
  i32 new_size = gb->size * 2;
  char *new_buf = malloc(new_size);

  // copy text to the left of the gap
  memcpy(new_buf, gb->buf, gb->gap_start);

  // copy text to the right of the gap
  i32 right_side_len = gb->size - gb->gap_end;
  memcpy(gb->buf + new_size - right_side_len, gb->buf + right_side_len, right_side_len);

  // free old buffer
  free(gb->buf);
  gb->buf = new_buf;
  gb->gap_end = new_size - right_side_len;
  gb->size = new_size;
}
#endif
