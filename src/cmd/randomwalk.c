#include <stdlib.h>

#include "base.h"
#include "draw.h"
#include "runtime-sdl.c"

#define WIDTH 800
#define HEIGHT 600

typedef struct Program {
  Point pos;
  Bitmap pen;
} Program;

int main(void) {
  MemoryArena *arena = arena_create(5 * MB);
  Bitmap pen = bitmap_create(arena, 5, 5);
  bitmap_fill(&pen, PALETTE_DARK_YELLOW);
  Point center = {WIDTH/2, HEIGHT/2};

  Program p = { .pos = center, .pen = pen };

  Runtime r = runtime_create(arena, STRING8("random-walk"), (Point){-1, -1}, WIDTH, HEIGHT, 1);
  r.context = (void *)&p;
  r.on_step = on_step;
  r.on_mouse_down = on_mouse_down;

  runtime_start(&r);
  runtime_destroy(&r);
  arena_destroy(arena);
}

void on_step(Runtime *runtime) {
  Program *p = (Program *)runtime->context;

  u32 direction = rand() % 4;
  u32 distance = 10;
  Point new_pos = {p->pos.x, p->pos.y};

  switch(direction) {
  case 0:  // UP
    new_pos.y -= distance;
    break;
  case 1:  // DOWN
    new_pos.y += distance;
    break;
  case 2:  // LEFT
    new_pos.x -= distance;
    break;
  case 3:  // RIGHT
    new_pos.x += distance;
  default:
    break;
  }

  if (new_pos.x < 0 || new_pos.x > runtime->width) { new_pos.x = p->pos.x; }
  if (new_pos.y < 0 || new_pos.y > runtime->height) { new_pos.y = p->pos.y; }

  draw_line(&(p->pen), &(runtime->screen), p->pos, new_pos, DRAWOP_STORE);
  p->pos = new_pos;
  runtime->needs_redisplay = true;
}

void on_mouse_down(Runtime *runtime) {
  Program *p = (Program *)runtime->context;

  p->pos = (Point){runtime->width / 2, runtime->height / 2};
  bitmap_clear(&(runtime->screen));
}
