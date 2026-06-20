#ifndef _FONT_H_
#define _FONT_H_

/*
 * font.h
 *
 * minimal (and naive) parser for bdf and hex fonts.
 */

#include <assert.h>
#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "base.h"
#include "draw.h"

typedef struct Glyph {
  Bitmap bitmap;
  i32 x_offset;  // add
  i32 y_offset;  // sub
  i64 x_advance;
  i64 y_advance;
} Glyph;

typedef struct Font {
  FT_Library lib;
  FT_Face face;
  i32 w;
  i32 h;
  Glyph glyphs[95];  // considering printable ascii rn
  i32 num_glyphs;
} Font;


Font font_create(MemoryArena *arena, String8 fontpath, i32 width, i32 height);
void font_destroy(Font *font);
void font_render_char(Font *font, char c, Bitmap *dst, Point pos, Color fg);
void font_render_text(Font *font, String8 txt, Bitmap *dst, Point pos, Color fg, Color bg);

Glyph __preload_glyph(MemoryArena *arena, FT_Face face, char c);

Font font_create(MemoryArena *arena, String8 fontpath, i32 width, i32 height) {
  FT_Library library;
  FT_Face face;

  if (FT_Init_FreeType(&library)) {
    fprintf(stderr, "could not initialize FreeType!\n");
    FT_Done_FreeType(library);
    abort();
  }

  // 2. load font
  if (FT_New_Face(library, fontpath.data, 0, &face)) {
    fprintf(stderr, "could not load font!");
    FT_Done_FreeType(library);
    abort();
  }

  FT_Set_Pixel_Sizes(face, width, height);
  Font f = {
    .lib = library,
    .face = face,
    .w = width,
    .h = height,
  };

  // preload glyphs
  for (u8 i=32; i < 127; i++) {
    f.glyphs[i - 32] = __preload_glyph(arena, f.face, (char)i);
  }
  return f;
}

void font_destroy(Font *font) {
  FT_Done_Face(font->face);
  FT_Done_FreeType(font->lib);
}

void font_render_char(Font *font, char c, Bitmap *dst, Point pos, Color fg) {
  MemoryArena *scratch = arena_create(2 * (font->w * font->h * sizeof(Color)));
  Bitmap mask = bitmap_create(scratch, font->w, font->h);
  bitmap_fill(&mask, fg);

  printf("Rendering char='%c' -- ", c);
  Glyph g = font->glyphs[(u8)c - 32];
  printf("found glyph -- ");

  //bitblt(&(g.bitmap), &mask, bitmap_rect(&(g.bitmap)), (Point){0, 0}, DRAWOP_AND);
  // printf("blitted mask\n");

  pos.x += g.x_offset;
  pos.y -= g.y_offset;
  // bitblt(&mask, dst, bitmap_rect(&mask), pos, DRAWOP_STORE);

  bitblt(&(g.bitmap), dst, bitmap_rect(&(g.bitmap)), pos, DRAWOP_STORE);
  arena_destroy(scratch);
}

Glyph __preload_glyph(MemoryArena *arena, FT_Face face, char c) {
  Color bg = 0x00000000;
  Color fg = 0xffffffff;

  printf("__preload_glyph: char='%c'\n", c);
  if (FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO | FT_LOAD_NO_HINTING)) {
    fprintf(stderr, "failed to load char='%c'\n", c);
    abort();
  }

  FT_GlyphSlot ft_glyph = face->glyph;
  FT_Bitmap *ft_bitmap  = &ft_glyph->bitmap;

  i32 ft_glyph_w = ft_bitmap->width;
  i32 ft_glyph_h = ft_bitmap->rows;
  i32 ft_picth   = ft_bitmap->pitch;

  // prepare our atlas/cache bitmap
  Bitmap glyph = bitmap_create(arena, ft_glyph_w, ft_glyph_h);

  for (i32 y=0; y < ft_glyph_h; y++) {
    for (i32 x=0; x < ft_glyph_w; x++) {

      u8 val   = ft_bitmap->buffer[(y * ft_picth) + (x / 8)];
      bool bit = (val >> (7 - (x % 8))) & 0x01;

      if (bit == 0) {
	printf("0"); // TODO debug only?
        glyph.pixels[PIXEL_INDEX(x, y, ft_glyph_w)] = bg;
	continue;
      }

      printf("1"); // TODO debug only?
      glyph.pixels[PIXEL_INDEX(x, y, ft_glyph_w)] = fg;
    }
    printf("\n"); // TODO debug only
  }

  return (Glyph){
    .bitmap = glyph,
    .x_offset = ft_glyph->bitmap_left,
    .y_offset = ft_glyph->bitmap_top,
  };
}

#endif
