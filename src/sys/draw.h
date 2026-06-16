#ifndef _DRAW_H_
#define _DRAW_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "base.h"

#define PIXEL_INDEX(x, y, w) (y * w) + x

typedef uint32_t Color;

typedef enum Palette {
  PALETTE_OPAQUE          = 0xFFFFFFFF,
  PALETTE_TRANSPARENT     = 0x00000000,
  PALETTE_BLACK           = 0x000000FF,
  PALETTE_WHITE           = 0xFFFFFFFF,
  PALETTE_RED             = 0xFF0000FF,
  PALETTE_GREEN           = 0x00FF00FF,
  PALETTE_BLUE            = 0x0000FFFF,
  PALETTE_CYAN            = 0x00FFFFFF,
  PALETTE_MAGENTA         = 0xFF00FFFF,
  PALETTE_YELLOW          = 0xFFFF00FF,
  PALETTE_PALE_YELLOW     = 0xFFFFAAFF,
  PALETTE_DARK_YELLOW     = 0xEEEE9EFF,
  PALETTE_DARK_GREEN      = 0x448844FF,
  PALETTE_PALE_GREEN      = 0xAAFFAAFF,
  PALETTE_MED_GREEN       = 0x88CC88FF,
  PALETTE_DARK_BLUE       = 0x000055FF,
  PALETTE_PALE_BLUE_GREEN = 0xAAFFFFFF,
  PALETTE_PALE_BLUE       = 0x0000BBFF,
  PALETTE_BLUE_GREEN      = 0x008888FF,
  PALETTE_GREY_GREEN      = 0x55AAAAFF,
  PALETTE_PALE_GREY_GREEN = 0x9EEEEEFF,
  PALETTE_YELLOW_GREEN    = 0x99994CFF,
  PALETTE_MED_BLUE        = 0x000099FF,
  PALETTE_GREY_BLUE       = 0x005DBBFF,
  PALETTE_PALE_GREY_BLUE  = 0x4993DDFF,
  PALETTE_PURPLE_BLUE     = 0x8888CCFF,

  PALETTE_NOT_A_COLOR     = 0xFFFFFF00,
  PALETTE_NO_FILL         = PALETTE_NOT_A_COLOR,
} Palette;

typedef struct Bitmap {
  i32 w;
  i32 h;
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

Bitmap bitmap_create(MemoryArena *arena, i32 width, i32 height);
Rect   bitmap_rect(Bitmap *b);
Color  bitmap_get_pixel(Bitmap *b, i32 x, i32 y);
void   bitmap_set_pixel(Bitmap *b, i32 x, i32 y, Color color);
void   bitmap_clear(Bitmap *b);
void   bitmap_fill(Bitmap *b, Color color);

void draw_line(Bitmap *brush, Bitmap *dst, Point from, Point to, DrawOp op);
void draw_line_clipped(Bitmap *brush, Bitmap *dst, Point from, Point to, Rect clip_rect, DrawOp op);
void draw_rect(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_rect_fill(Bitmap *brush, Bitmap *dst, Point origin, Point corner, Rect clip_rect, DrawOp op);
void draw_circle(Bitmap *brush, Bitmap *dst, Point center, i32 radius, Rect clip_rect, DrawOp op);
void draw_circle_fill(Bitmap *brush, Bitmap *dst, Point center, i32 radius, Rect clip_rect, DrawOp op);

void bitblt(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, DrawOp op);
void bitblt_clipped(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op);
void __clip(Bitmap *src, Bitmap *dst, Rect *src_rect, Point *at_pos, Rect clip_rect);
void __copy_bits(Bitmap *src, Bitmap *dst, Rect src_rect, Point pos, DrawOp op);
void __merge(Bitmap *src, Bitmap *dst, i32 src_x, i32 src_y, i32 dst_x, i32 dst_y, i32 n, DrawOp op);
i8   __sign(i32 val);

Bitmap bitmap_create(MemoryArena *arena, i32 width, i32 height) {
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

Color  bitmap_get_pixel(Bitmap *b, i32 x, i32 y) {
  i32 index = PIXEL_INDEX(x, y, b->w);
  return b->pixels[index];
}

void   bitmap_set_pixel(Bitmap *b, i32 x, i32 y, Color color) {
  i32 index = PIXEL_INDEX(x, y, b->w);
  b->pixels[index] = color;
}

void   bitmap_clear(Bitmap *b) {
  memset(b->pixels, 0, b->w * b->h * sizeof(Color));
}

void   bitmap_fill(Bitmap *b, Color color) {
  // this used to be a single loop, but it was close to 20x slower than
  // the bitmap_clear implementation.
  // even with loop unrolling i only managed to get it to 20x slower than
  // bitmap_clear.
  //
  // can't say i'm super into this, but it's comparable in performance to
  // bitmap_clear. a bit slower, but on the same order of magnitude.

  // fill one row with color
  i32 row[b->w];
  for (i32 x=0; x < b->w; x++) {
    row[x] = color;
  }

  // memcopy it to every row
  for (i32 y=0; y < b->h; y++) {
    memcpy(b->pixels + (b->w * y), row, b->w * sizeof(Color));
  }
}

void draw_line(Bitmap *brush, Bitmap *dst, Point from, Point to, DrawOp op) {
  draw_line_clipped(brush, dst, from, to, bitmap_rect(dst), op);
}

void draw_line_clipped(Bitmap *brush, Bitmap *dst, Point from, Point to, Rect clip_rect, DrawOp op) {
  i32 from_x = from.x;
  i32 from_y = from.y;
  i32 to_x = to.x;
  i32 to_y = to.y;

  bool is_forward = ((from_y == to_y) && (from_x < to_x)) || (from_y < to_y);
  if (!is_forward) {
    i32 tmp_x = from_x;
    i32 tmp_y = from_y;

    from_x = to_x;
    to_x = tmp_x;

    from_y = to_y;
    to_y = tmp_y;
  }

  Rect src_rect = bitmap_rect(brush);
  Point at = {from_x, from_y};

  i32 x_delta = to_x - from_x;
  i32 y_delta = to_y - from_y;

  i32 dx = __sign(x_delta);
  i32 dy = __sign(y_delta);
  i32 px = labs(y_delta);
  i32 py = labs(x_delta);

  if (py > px) {
    // line is more horizontal
    i32 p = (i32)(py / 2);
    for (i32 i=0; i < py; i++) {
      at.x += dx;
      p -= px;

      if (p < 0) {
	at.y += dy;
	p += py;
      }

      if (i < py) {
	bitblt_clipped(brush, dst, src_rect, at, clip_rect, op);
      }
    }
  } else {
    // line is more vertical
    i32 p = (i32)(px / 2);
    for (i32 i = 0; i < px; i++) {
      at.y += dy;
      p -= py;

      if (p < 0) {
	at.x += dx;
	p += px;
      }

      if (i < px) {
	bitblt_clipped(brush, dst, src_rect, at, clip_rect, op);
      }
    }
  }

  // draw the first point
  Point first_point = is_forward ? from : to;
  bitblt_clipped(brush, dst, src_rect, at, clip_rect, op);
}

void bitblt(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, DrawOp op) {
  bitblt_clipped(src, dst, src_rect, at_pos, bitmap_rect(dst), op);
}

void bitblt_clipped(Bitmap *src, Bitmap *dst, Rect src_rect, Point at_pos, Rect clip_rect, DrawOp op) {
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

  i32 src_rect_w = src_rect->corner.x - src_rect->origin.x;
  if ((at_pos->x + src_rect_w) > clip_rect.corner.x) {
    // the src_rect's right side is outside the clip_rect's right side
    // so we must adjust the src_rect's right side so we only copy the
    // src pixels that will fit into the clip_Rect.
    // 1. get the width of the clip_rect
    // 2. get the width of the src_rect (which has been adjusted above somewhat)
    // 3. if src_rect width is < clip_rect width do nothing
    // 4. otherwise get the difference and adjust src_rect left side by this amount.
    i32 clip_rect_w = clip_rect.corner.x - clip_rect.origin.x;
    i32 diff = (at_pos->x + src_rect_w) - clip_rect_w;
    if (diff > 0) {
      i32 delta_x = diff > src_rect_w ? src_rect_w : diff;
      src_rect->corner.x -= delta_x;
    }
  }

  // in y (we do the same but for y and height)
  if (at_pos->y <= clip_rect.origin.y) {
    src_rect->origin.y += (clip_rect.origin.y - at_pos->y);
    at_pos->y = clip_rect.origin.y;
  }

  i32 src_rect_h = src_rect->corner.y - src_rect->origin.y;
  if ((at_pos->y + src_rect_h) > clip_rect.corner.y) {
    i32 clip_rect_h = clip_rect.corner.y - clip_rect.origin.y;
    i32 diff = (at_pos->y + src_rect_h) - clip_rect_h;
    i32 delta_y = diff > src_rect_h ? src_rect_h : diff;
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
  i32 dst_y = at_pos.y;

  for (i32 src_y = src_rect.origin.y; src_y < src_rect.corner.y; src_y++) {
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
void __merge(Bitmap *src, Bitmap *dst, i32 src_x, i32 src_y, i32 dst_x, i32 dst_y, i32 n, DrawOp op) {
    for (i32 i=0; i < n; i++) {
      i32 src_i = PIXEL_INDEX(src_x, src_y, src->w);
      i32 dst_i = PIXEL_INDEX(dst_x, dst_y, dst->w);

      switch(op) {
      case DRAWOP_STORE:
	dst->pixels[dst_i] = src->pixels[src_i];
        break;
      case DRAWOP_STORE_INVERT:
	dst->pixels[dst_i] = ~src->pixels[src_i];
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

i8 __sign(i32 val) {
  if (val > 0) return 1;
  if (val < 0) return -1;
  return 0;
}
#endif
