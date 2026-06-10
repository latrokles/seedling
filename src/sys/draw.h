#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "base.h"

typedef uint32_t Color;

typedef struct Bitmap {
  u64 w;
  u64 h;
  Color *pixels;
} Bitmap;

typedef enum {
  DRAWOP_STORE,        // dst = src
  DRAWOP_STORE_INVERT, // dst = ~src
  DRAWOP_OR,           // dst = dst | src
  DRAWOP_AND,          // dst = dst & src
  DRAWOP_XOR,          // dst = dst ^ src
  DRAWOP_CLR,          // dst = dst & ~src
} DrawOp;

Bitmap bitmap_create(MemoryArena *arena, u64 width, u64 height);
void draw_fill(Bitmap *b, Color color);
void draw_line(Bitmap *brush, Bitmap *dst, Point from, Point to, Rect clip_rect, DrawOp op);
void draw_rect(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_rect_fill(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_circle(Bitmap *brush, Bitmap *dst, Point center, u64 radius, Rect clip_rect, DrawOp op);
void draw_circle_fill(Bitmap *brush, Bitmap *dst, Point center, u64 radius, Rect clip_rect, DrawOp op);
void bitblt(Bitmap *src, Bitmap *dst, Rect src_rect, Point pos, Rect clip_rect, DrawOp op);
void __clip(Bitmap *src, Bitmap *dst, Rect src_rect, Point *pos, Rect clip_rect);
void __copy_bits(Bitmap *src, Bitmap *dst, Point pos, DrawOp op);
void __merge(Bitmap *src, Bitmap *dst, u64 src_x, u64 src_y, u64 dst_x, u64 dst_y, usize n, DrawOp op);


Bitmap bitmap_create(MemoryArena *arena, u64 width, u64 height) {
  Color *pixels = arena_push(arena, (width * height * sizeof(Color)));

  return (Bitmap){
    .w = width,
    .h = height,
    .pixels = pixels,
  };
}


#endif
