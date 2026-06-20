#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "base.h"
#include "draw.h"
#include "runtime-sdl.c"
#include "font.h"

#define WIDTH 800
#define HEIGHT 608

#define CELL_SIZE 16

typedef struct TextWriter {
  Font font;
  Color text_color;
  Point cursor;
  Bitmap pen;
} TextWriter;

void draw_grid(Bitmap *pen, Bitmap *screen);
void render_char(FT_Face face, char c, Bitmap *b, Point pos, Color bg, Color fg);

int main(int argc, char *argv[]) {
  assert(argc == 2);
  char *fontpath = argv[1];
  i32 fontpath_len = strlen(fontpath);

  MemoryArena *arena = arena_create(10 * MB);
  Runtime r = runtime_create(arena, STRING8("text input and rendering"), (Point){-1, -1}, WIDTH, HEIGHT, 1);

  TextWriter ctx = {
    .font = font_create(arena, string8_from_charbuf(arena, fontpath, fontpath_len), CELL_SIZE, CELL_SIZE),
    .text_color = PALETTE_DARK_YELLOW,
    .cursor = (Point){0, CELL_SIZE},
    .pen = bitmap_create(arena, 1, 1),
  };
  bitmap_fill(&(ctx.pen), PALETTE_BLUE);

  draw_grid(&(ctx.pen), &(r.screen));

  r.context = (void *)&ctx;
  r.on_text_in = on_text_in;
  runtime_start(&r);
  runtime_destroy(&r);
  arena_destroy(arena);
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

void on_text_in(Runtime *runtime, String8 s) {
  TextWriter *ctx = (TextWriter *)(runtime->context);

  printf("invoked on_text_in, s.data=%s, s.length=%lu\n", s.data, s.length);
  for (u64 i=0; i < s.length; i++) {
    printf("loop...");
    font_render_char(&(ctx->font), s.data[i], &(runtime->screen), ctx->cursor, ctx->text_color);
    ctx->cursor.x += ctx->font.w;
  }
  printf("\n");
}
