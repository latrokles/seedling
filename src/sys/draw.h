#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "base.h"

#define PIXEL_INDEX(x, y, w) (y * w) + x

typedef uint32_t Color;

typedef struct Bitmap {
  i64 w;
  i64 h;
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

Bitmap bitmap_create(MemoryArena *arena, i64 width, i64 height);
Rect   bitmap_rect(Bitmap *b);
Color  bitmap_get_pixel(Bitmap *b, i64 x, i64 y);
void   bitmap_set_pixel(Bitmap *b, i64 x, i64 y, Color color);
void   bitmap_fill(Bitmap *b, Color color);

void draw_line(Bitmap *brush, Bitmap *dst, Point from, Point to, Rect clip_rect, DrawOp op);
void draw_rect(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_rect_fill(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_circle(Bitmap *brush, Bitmap *dst, Point center, i64 radius, Rect clip_rect, DrawOp op);
void draw_circle_fill(Bitmap *brush, Bitmap *dst, Point center, i64 radius, Rect clip_rect, DrawOp op);

void bitblt(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op);
void bitblt_clip(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op);
void __clip(Bitmap *src, Bitmap *dst, Rect *src_rect, Point *at_pos, Rect clip_rect);
void __copy_bits(Bitmap *src, Bitmap *dst, Rect src_rect, Point pos, DrawOp op);
void __merge(Bitmap *src, Bitmap *dst, i64 src_x, i64 src_y, i64 dst_x, i64 dst_y, i64 n, DrawOp op);
i8   __sign(i64 val);

Bitmap bitmap_create(MemoryArena *arena, i64 width, i64 height) {
  Color *pixels = arena_push(arena, (width * height * sizeof(Color)));

  return (Bitmap){
    .w = width,
    .h = height,
    .pixels = pixels,
  };
}

Rect   bitmap_rect(Bitmap *b) {
  Point origin = {0, 0};
  Point corner = {b->w, b->h};
  return (Rect){origin, corner};
}

Color  bitmap_get_pixel(Bitmap *b, i64 x, i64 y) {
  i64 index = PIXEL_INDEX(x, y, b->w);
  return b->pixels[index];
}

void   bitmap_set_pixel(Bitmap *b, i64 x, i64 y, Color color) {
  i64 index = PIXEL_INDEX(x, y, b->w);
  b->pixels[index] = color;
}

void   bitmap_fill(Bitmap *b, Color color) {
  for (i64 i=0; i< (b->w * b->h); i++) {
    b->pixels[i] = color;
  }
}

void bitblt(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op) {
  bitblt_clip(src, dst, src_rect, at_pos, bitmap_rect(dst), op);
}

void bitblt_clip(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op) {
  __clip(src, dst, &src_rect, &at_pos, clip_rect);
  __copy_bits(src, dst, src_rect, at_pos, op);
}

/*
  Adjust the dimensions of source and clipping rectangle to fit within the src and dst bitmaps so that
  we avoid doing extra work for regions of the bitmap that won't be displayed.
*/
void __clip(Bitmap *src, Bitmap *dst, Rect *src_rect, Point *at_pos, Rect clip_rect) {
  // if clipping rectangle `clip_rect` is outside of destination bitmap `dst` we discard the out of bands region
  if (clip_rect.origin.x < 0) clip_rect.origin.x = 0; // left edge
  if (clip_rect.origin.y < 0) clip_rect.origin.y = 0; // top edge

  if (clip_rect.corner.x > dst->w) clip_rect.corner.x = dst->w; // right edge
  if (clip_rect.corner.y > dst->h) clip_rect.corner.y = dst->h; // bottom edge

  // clip and adjust src_rect according to clip_rect
  // in x
  if (at_pos->x <= clip_rect.origin.x) {
    // the src_rect's left side is outside the clip_rect's left side
    // so we must adjust the src_rect's left side so we only copy the
    // src pixels that overlap with the clip_rect.
    // 1. move src_rect left side by the difference between at.x and clip_rect.origin.x
    // 2. adjust at.x to clip_rect.origin.x so we copy bits right at the clip_rect's origin.
    src_rect->origin.x += (clip_rect.origin.x - at_pos->x);
    at_pos->x = clip_rect.origin.x;
  }

  i64 src_rect_w = src_rect->corner.x - src_rect->origin.x;
  if ((at_pos->x + src_rect_w) > clip_rect.corner.x) {
    // the src_rect's right side is outside the clip_rect's right side
    // so we must adjust the src_rect's right side so we only copy the
    // src pixels that will fit into the clip_Rect.
    // 1. get the width of the clip_rect
    // 2. get the width of the src_rect (which has been adjusted above somewhat)
    // 3. if src_rect width is < clip_rect width do nothing
    // 4. otherwise get the difference and adjust src_rect left side by this amount.
    i64 clip_rect_w = clip_rect.corner.x - clip_rect.origin.x;
    i64 diff = (at_pos->x + src_rect_w) - clip_rect_w;
    if (diff > 0) {
      i64 delta_x = diff > src_rect_w ? src_rect_w : diff;
      src_rect->corner.x -= delta_x;
    }
  }

  // in y (we do the same but for y and height)
  if (at_pos->y <= clip_rect.origin.y) {
    src_rect->origin.y += (clip_rect.origin.y - at_pos->y);
    at_pos->y = clip_rect.origin.y;
  }

  i64 src_rect_h = src_rect->corner.y - src_rect->origin.y;
  if ((at_pos->y + src_rect_h) > clip_rect.corner.y) {
    i64 clip_rect_h = clip_rect.corner.y - clip_rect.origin.y;
    i64 diff = (at_pos->y + src_rect_h) - clip_rect_h;
    i64 delta_y = diff > src_rect_h ? src_rect_h : diff;
    if (diff > 0) src_rect->corner.y -= delta_y;
  }

  if (src == NULL) return;

  if (src_rect->origin.x < 0) {
    at_pos->x -= src_rect->origin.x;
    src_rect->origin.x = 0;
  }
  if (src_rect->corner.x > src->w) src_rect->corner.x = src->w;

  if (src_rect->origin.y < 0) {
    at_pos->y -= src_rect->origin.y;
    src_rect->origin.y = 0;
  }
  if (src_rect->corner.y > src->h) src_rect->corner.y = src->h;
}

void __copy_bits(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, DrawOp op) {
  i64 dst_y = at_pos.y;

  for (i64 src_y = src_rect.origin.y; src_y < src_rect.corner.y; src_y++) {
    __merge(src,
	    dst,
	    src_rect.origin.x,
	    src_y,
	    at_pos.x,
	    dst_y,
	    src_rect.corner.x - src_rect.origin.x,
	    op);
    dst_y++;
  }
}

/*
 Copy `n` pixels from `src` coordinates (`src_x`, `src_y`) onto `dst` coordinates (`dst_x`, `dst_y`)
 while applying the DrawOp operation.
*/
void __merge(Bitmap *src, Bitmap *dst, i64 src_x, i64 src_y, i64 dst_x, i64 dst_y, i64 n, DrawOp op) {
    for (i64 i=0; i < n; i++) {
      i64 src_i = PIXEL_INDEX(src_x, src_y, src->w);
      i64 dst_i = PIXEL_INDEX(dst_x, dst_y, dst->w);

      switch(op) {
      case DRAWOP_STORE:
	dst->pixels[dst_i] = src->pixels[src_i];
        break;
      case DRAWOP_OR:
	dst->pixels[dst_i] |= src->pixels[src_i];
        break;
      case DRAWOP_AND:
        dst->pixels[dst_i] &= src->pixels[src_i];
        break;
      case DRAWOP_XOR:
        dst->pixels[dst_i] ^= src->pixels[src_i];
        break;
      case DRAWOP_CLR:
        dst->pixels[dst_i] = 0x00000000;
        break;
      default:
        break;
      }

      src_x++;
      dst_x++;
  }
}

i8 __sign(i64 val) {
  if (val > 0) return 1;
  if (val < 0) return -1;
  return 0;
}
#endif
