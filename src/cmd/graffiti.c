/*
  graffiti.c

  simple handwritten recognition software.

*/

#include "base.h"
#include "runtime-sdl.c"

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

  while (runtime.is_executing) {
    runtime_update(&runtime);
    if (runtime.mouse_l) {
      printf("mouse left button is down\n");
    }
  }

  runtime_destroy(&runtime);
  arena_destroy(arena);
}
