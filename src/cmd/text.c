#include <ft2build.h>
#include FT_FREETYPE_H

#include "base.h"
#include "draw.h"
#include "runtime-sdl.c"

#define WIDTH 800
#define HEIGHT 608

#define CELL_SIZE 32

void draw_grid(Bitmap *pen, Bitmap *screen);
void render_char(FT_Face face, char c, Bitmap *b, Point pos, Color bg, Color fg);

int main(int argc, char *argv[]) {
  assert(argc == 2);
  char *fontpath = argv[1];

  FT_Library library;
  FT_Face face;

  MemoryArena *arena = arena_create(10 * MB);
  Runtime r = runtime_create(arena, STRING8("random-walk"), (Point){-1, -1}, WIDTH, HEIGHT, 1);
  Bitmap pen = bitmap_create(arena, 1, 1);
  bitmap_fill(&pen, PALETTE_BLUE);

  // 1. init FT
  if (FT_Init_FreeType(&library)) {
    fprintf(stderr, "could not initialize FreeType!\n");
    FT_Done_FreeType(library);
    return -1;
  }

  // 2. load font
  if (FT_New_Face(library, fontpath, 0, &face)) {
    fprintf(stderr, "could not load font!");
    FT_Done_FreeType(library);
    return -1;
  }

  // 3. Set font size (width and height in 1/64th of a point)
  // FT_Set_Char_Size(face, 0, 10 * 64, 300, 300);
  // FT_Set_Char_Size(face, 0, 16 * 64, 300, 300);
  FT_Set_Pixel_Sizes(face, 0, CELL_SIZE);

  Color c = 0xffffff00;
  u8 red   = (c >> 24) & 0xff;
  u8 green = (c >> 16) & 0xff;
  u8 blue  = (c >>  8) & 0xff;
  u8 alpha =  c        & 0xff;
  printf("r=%d, g=%d, b=%d, alpha=%d\n", red, green, blue, alpha);

  // 3. render
  draw_grid(&pen, &(r.screen));

  render_char(face, 'B', &(r.screen), (Point){0, CELL_SIZE}, c, PALETTE_YELLOW);
  render_char(face, 'b', &(r.screen), (Point){CELL_SIZE * 2, CELL_SIZE}, PALETTE_BLACK, PALETTE_YELLOW);

  runtime_start(&r);
  runtime_destroy(&r);
  arena_destroy(arena);

  FT_Done_Face(face);
  FT_Done_FreeType(library);
}

void draw_grid(Bitmap *pen, Bitmap *screen) {
  i32 cols = WIDTH / CELL_SIZE;
  i32 rows = HEIGHT / CELL_SIZE;

  for (i32 r=0; r < rows; r++) {
    for (i32 c=0; c < cols; c++) {
      draw_line(pen, screen, (Point){CELL_SIZE*c, 0}, (Point){CELL_SIZE*c, HEIGHT}, DRAWOP_STORE);
    }
    draw_line(pen, screen, (Point){0, CELL_SIZE*r}, (Point){WIDTH, CELL_SIZE*r}, DRAWOP_STORE);
  }
}

void render_char(FT_Face face, char c, Bitmap *b, Point pos, Color bg, Color fg) {
  printf("render char='%c' at pos=(%d, %d)\n", c, pos.x, pos.y);

  if (FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO | FT_LOAD_NO_HINTING)) {
    fprintf(stderr, "Could not load char '%c'\n", c);
    return;
  }
  FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

  MemoryArena *scratch = arena_create(0.5 * MB);

  FT_GlyphSlot glyph = face->glyph;
  FT_Bitmap *bitmap = &glyph->bitmap;

  i32 glyph_w = bitmap->width;
  i32 glyph_h = bitmap->rows;
  i32 pitch   = bitmap->pitch;

  Bitmap rendered_glyph = bitmap_create(scratch, glyph_w, glyph_h);
  bitmap_fill(&rendered_glyph, bg);

  for (i32 y=0; y < glyph_h; y++) {
    if (y < 0) {
      printf("out of bounds: pixel_y=%d, bitmap=(w:%d, h%d)\n", y, rendered_glyph.w, rendered_glyph.h);
      continue;  // out of bounds check
    }

    for (i32 x=0; x < glyph_w; x++) {
      if (x < 0 || x >= rendered_glyph.w) {
	printf("out of bounds: pixel=(%d, %d), bitmap=(w:%d, h%d)\n", x, y, rendered_glyph.w, rendered_glyph.h);
	continue;
      }

      u8 val = bitmap->buffer[(y * pitch) + (x / 8)];
      bool bit = (val >> (7 - (x % 8))) & 0x01;

      if (bit == 0) {
	printf("0");
	continue;
      }

      printf("1");
      rendered_glyph.pixels[PIXEL_INDEX(x, y, glyph_w)] = fg;
    }
    printf("\n");
  }

  pos.x -= glyph->bitmap_left;
  pos.y -= glyph->bitmap_top;
  bitblt(&rendered_glyph, b, bitmap_rect(&rendered_glyph), pos, DRAWOP_STORE);
  arena_destroy(scratch);
}
