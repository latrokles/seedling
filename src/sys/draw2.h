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

Bitmap bitmap_create(MemoryArena *arena, i32 width, i32 height);
i32    bitmap_count_pixels(Bitmap *b);
Color  bitmap_get_pixel(Bitmap *b, i32 x, i32 y);
void   bitmap_set_pixel(Bitmap *b, i32 x, i32 y, Color color);
void   bitmap_clear(Bitmap *b);
void   bitmap_fill(Bitmap *b, Color color);

void draw_line(Bitmap *b, Point start, Point end, Color color);

Bitmap bitmap_create(MemoryArena *arena, i32 width, i32 height) {
  Color *pixels = arena_push(arena, (width * height * sizeof(Color)));

  return (Bitmap){
    .w = width,
    .h = height,
    .pixels = pixels,
  };
}

i32    bitmap_count_pixels(Bitmap *b) {
  return b->w * b->h;
}

Color  bitmap_get_pixel(Bitmap *b, i32 x, i32 y) {
  i32 index = PIXEL_INDEX(x, y, b->w);
  return b->pixels[index];
}

void   bitmap_set_pixel(Bitmap *b, i32 x, i32 y, Color color) {
  if ((x < 0) || (x >= b->w)) return;
  if ((y < 0) || (y >= b->h)) return;

  i32 index = PIXEL_INDEX(x, y, b->w);
  b->pixels[index] = color;
}

void   bitmap_clear(Bitmap *b) {
  memset(b->pixels, 0x0, b->w * b->h * sizeof(Color));
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

void draw_line(Bitmap *b, Point start, Point end, Color color) {
  i32 dx = abs(end.x - start.x);
  i32 dy = abs(end.y - start.y);

  i32 sx = start.x < end.x ? 1 : -1;
  i32 sy = start.y < end.y ? 1 : -1;

  i32 err = dx - dy;

  while(true) {
    b->pixels[PIXEL_INDEX(start.x, start.y, b->w)] = color;

    if (start.x == end.x && start.y == end.y) {
      break;
    }

    int e2 = 2 * err;
    if (e2 >= -dy) {
      err     -= dy;
      start.x += sx;
    }

    if (e2 <= dx) {
      err     += dx;
      start.y += sy;
    }
  }
}

void draw_line2(Bitmap *b, Point start, Point end, Color color) {
  i32 x0 = start.x, y0 = start.y;
  i32 x1 = end.x, y1 = end.y;

  i32 dx = x1 - x0;
  i32 dy = y1 - y0;

  i32 x_step = 0;
  i32 y_step = 0;

  if (dy < 0) {
    dy     = -dy;
    y_step = -1;
  } else {
    y_step = 1;
  }

  if (dx < 0) {
    dx     = -dx;
    x_step = -1;
  } else {
    x_step = 1;
  }

  dx = 2 * dx;
  dy = 2 * dy;

  b->pixels[PIXEL_INDEX(x0, y0, b->w)] = color;

  i32 fraction;
  if (dx > dy) {
    fraction = 2 * dy - dx;
    while (x0 != x1) {
      if (fraction >= 0) {
	y0       += y_step;
	fraction -= dx;
      }

      x0       += x_step;
      fraction += dy;
      b->pixels[PIXEL_INDEX(x0, y0, b->w)] = color;
    }
  } else {
    fraction = 2 * dx - dy;
    while (y0 != y1) {
      if (fraction >= 0) {
	x0       += x_step;
	fraction -= dy;
      }

      y0       += y_step;
      fraction += dx;
      b->pixels[PIXEL_INDEX(x0, y0, b->w)] = color;
    }
  }
}


#endif
