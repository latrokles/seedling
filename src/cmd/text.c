#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "base.h"
#include "draw.h"
#include "runtime-sdl.c"
#include "font.h"
#include "buffer.h"

#define FONT_SIZE 16
#define COLS 100
#define ROWS  50

typedef struct TextWriter {
  Font font;
  Color text_color;
  Point cursor;
  GapBuffer buffer;
  Bitmap pen;
  bool dirty;
} TextWriter;

void render_char(FT_Face face, char c, Bitmap *b, Point pos, Color bg, Color fg);

int main(int argc, char *argv[]) {
  assert(argc == 2);
  char *fontpath = argv[1];
  i32 fontpath_len = strlen(fontpath);

  int width = COLS * FONT_SIZE;
  int height = ROWS * FONT_SIZE;
  MemoryArena *arena = arena_create(10 * MB);
  Runtime r = runtime_create(arena, STRING8("text input and rendering"), (Point){-1, -1}, width, height, 1);

  TextWriter ctx = {
    .font = font_create(arena, string8_from_charbuf(arena, fontpath, fontpath_len), FONT_SIZE, FONT_SIZE),
    .text_color = PALETTE_DARK_YELLOW,
    .cursor = (Point){0, FONT_SIZE},
    .buffer = buffer_create(),
    .pen = bitmap_create(arena, 1, 1),
    .dirty = false,
  };
  bitmap_fill(&(ctx.pen), PALETTE_BLUE);

  r.context = (void *)&ctx;
  r.on_step = on_step;
  r.on_text_in = on_text_in;
  r.on_key_down = on_key_down;
  runtime_start(&r);
  runtime_destroy(&r);
  arena_destroy(arena);
}

void on_step(Runtime *runtime) {
  TextWriter *ctx = (TextWriter *)(runtime->context);

  // don't render if buffer has not changed.
  if (!ctx->dirty) {
    return;
  }

  // clear the screen and reset drawing cursor
  bitmap_clear(&(runtime->screen));

  // TODO this should be a local since it's now only used for rendering
  ctx->cursor.x = 0;
  ctx->cursor.y = FONT_SIZE;


  // draw buffer contents
  // TODO: encapsulate a little

  // draw text to the left of the gap
  for (i32 li=0; li < ctx->buffer.gap_start; li++) {
    if (ctx->buffer.buf[li] == '\n') {
      // found new line, go to start of the next line
      ctx->cursor.x = 0;
      ctx->cursor.y += FONT_SIZE;
      continue;
    }

    Glyph g = font_render_char(&(ctx->font), ctx->buffer.buf[li], &(runtime->screen), ctx->cursor, ctx->text_color);
    ctx->cursor.x += g.x_advance + g.x_bearing_h;
  }

  // print data to the right of the gap
  for (i32 ri=ctx->buffer.gap_end; ri < ctx->buffer.size; ri++) {
    if (ctx->buffer.buf[ri] == '\n') {
      // found new line, go to start of the next line
      ctx->cursor.x = 0;
      ctx->cursor.y += FONT_SIZE;
      continue;
    }

    Glyph g = font_render_char(&(ctx->font), ctx->buffer.buf[ri], &(runtime->screen), ctx->cursor, ctx->text_color);
    ctx->cursor.x += g.x_advance + g.x_bearing_h;
  }

  ctx->dirty = false;
}

void on_text_in(Runtime *runtime, String8 s) {
  TextWriter *ctx = (TextWriter *)(runtime->context);

  printf("invoked on_text_in, s.data=%s, s.length=%lu\n", s.data, s.length);
  for (u64 i=0; i < s.length; i++) {
    printf("loop...");
    buffer_insert(&(ctx->buffer), s.data[i]);
  }
  printf("\n");
  ctx->dirty = true;
}

void on_key_down(Runtime *runtime) {
  TextWriter *ctx = (TextWriter *)(runtime->context);

  if (runtime->keyboard.keys[K_RETURN]) {
    buffer_insert(&(ctx->buffer), '\n');
    ctx->dirty = true;
  }

  if (runtime->keyboard.keys[K_BACKSPACE]) {
    buffer_backspace(&(ctx->buffer));
    ctx->dirty = true;
  }

  if (runtime->keyboard.keys[K_ESCAPE]) {
    buffer_print(&(ctx->buffer));
  }
}
