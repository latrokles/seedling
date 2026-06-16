/*
  graffiti.c

  simple handwritten recognition software.

*/
#include <stdbool.h>

#include "base.h"
#include "draw.h"
#include "runtime-sdl.c"

#define WIDTH 800
#define HEIGHT 600

typedef struct PointList {
  Point **points;
  u64     length;
  u64     capacity;
} PointList;

PointList  point_list_create(MemoryArena *arena, u64 capacity);
Point     *point_list_get(PointList *list, u64 index);
void       point_list_add(MemoryArena *arena, PointList *list, Point *p);
void       point_list_clear(PointList *list);

typedef struct Graffiti {
  Bitmap pen;
} Graffiti;

int main(void) {
  MemoryArena *arena = arena_create(200 * MB);

  Bitmap pen = bitmap_create(arena, 10, 10);
  bitmap_fill(&pen, PALETTE_YELLOW);

  Runtime runtime = runtime_create(arena,
				   STRING8("graffiti"),
				   (struct Point){ 0, 0},
				   WIDTH,
				   HEIGHT,
				   1);

  Graffiti g = { .pen = pen };

  runtime.context = (void *)&g;
  runtime.on_mouse_down = on_mouse_down;
  runtime.on_mouse_motion = on_mouse_motion;
  runtime_start(&runtime);

  runtime_destroy(&runtime);
  arena_destroy(arena);
}

void on_mouse_down(Runtime *runtime) {
  if (runtime->mouse_r) {
    bitmap_fill(&(runtime->screen), PALETTE_BLACK);
  }
}

void on_mouse_motion(Runtime *runtime) {
  if (runtime->mouse_l) {
    Bitmap pen = ((Graffiti *)runtime->context)->pen;
    draw_line(&pen, &(runtime->screen), runtime->mouse_prev, runtime->mouse_curr, DRAWOP_STORE);
  }
}

PointList  point_list_create(MemoryArena *arena, u64 capacity) {
  Point **points = arena_push(arena, sizeof(Point*) * capacity);

  return (PointList){
    .points   = points,
    .length   = 0,
    .capacity = capacity,
  };
}
Point     *point_list_get(PointList *list, u64 index) {
  if (index > list->length) {
    return NULL;
  }

  return list->points[index];
}

void       point_list_add(MemoryArena *arena, PointList *list, Point *p) {
  // printf("adding point {x=%ld, y=%ld}\n", p->x, p->y);
  if (list->length + 1 > (u64)(list->capacity * 0.75)) {
    list->points = arena_grow(arena, list->points, list->capacity, (list->capacity * 2));
  }
  list->points[list->length] = p;
  list->length++;
}

void       point_list_clear(PointList *list) {
  if (list->length == 0) return;
  list->length = 0;
}
