/*
  graffiti.c

  simple handwritten recognition software.

*/

#include "base.h"
#include "runtime-sdl.c"
#include "draw.h"

#define WIDTH 600
#define HEIGHT 400


int main(void) {
  MemoryArena *arena = arena_create(200 * MB);

  Runtime runtime = runtime_create(arena,
				   STRING8("graffiti"),
				   (struct Point){ 0, 0},
				   WIDTH,
				   HEIGHT,
				   1);
  runtime_start(&runtime);
  Bitmap p = bitmap_create(arena, 10, 10);
  bitmap_fill(&p, PALETTE_PALE_YELLOW);

  Point p0 = {};
  Point p1 = {};
  while (runtime.is_executing) {
    runtime_update(&runtime);
    if (runtime.mouse_l) {
      p0.x = runtime.mouse_px;
      p0.y = runtime.mouse_py;
      p1.x = runtime.mouse_x;
      p1.y = runtime.mouse_y;
      draw_line(&p, &(runtime.screen), p0, p1, DRAWOP_STORE);
      runtime_redisplay(&runtime);
    }
  }

  runtime_destroy(&runtime);
  arena_destroy(arena);
}
