/*
  graffiti.c

  simple handwritten recognition software.

*/
#include <stdbool.h>

#include "base.h"
#include "runtime-sdl.c"
#include "draw.h"

#define WIDTH 600
#define HEIGHT 400

typedef struct PointList {
  Point **points;
  u64     length;
  u64     capacity;
} PointList;

PointList  point_list_create(MemoryArena *arena, u64 capacity);
Point     *point_list_get(PointList *list, u64 index);
void       point_list_add(MemoryArena *arena, PointList *list, Point *p);
void       point_list_clear(PointList *list);

void draw_points(Bitmap *pen, Bitmap *surface, PointList *points) {
  for(u64 i=0; i < points->length; i++) {
    Point *p = point_list_get(points, i);
    printf("drawing point {%ld, %ld}\n", p->x, p->y);
    bitblt(pen, surface, bitmap_rect(pen), (Point){p->x, p->y}, DRAWOP_STORE);
  }
}

int main(void) {
  MemoryArena *arena = arena_create(200 * MB);

  Runtime runtime = runtime_create(arena,
				   STRING8("graffiti"),
				   (struct Point){ 0, 0},
				   WIDTH,
				   HEIGHT,
				   1);

  runtime_start(&runtime);
  Bitmap p = bitmap_create(arena, 1, 1);
  bitmap_fill(&p, PALETTE_PALE_YELLOW);

  Bitmap point_pen = bitmap_create(arena, 10, 10);
  bitmap_fill(&point_pen, PALETTE_PALE_YELLOW);

  PointList points = point_list_create(arena, 600);
  Point p0 = {};
  Point p1 = {};
  while (runtime.is_executing) {
    runtime_update(&runtime);
    if (runtime.mouse_l) {
      p0.x = runtime.mouse_px;
      p0.y = runtime.mouse_py;
      p1.x = runtime.mouse_x;
      p1.y = runtime.mouse_y;

      //point_list_add(arena, &points, &(Point){runtime.mouse_x, runtime.mouse_y});
      //draw_points(&point_pen, &(runtime.screen), &points);
      draw_line(&p, &(runtime.screen), p0, p1, DRAWOP_STORE);
      runtime_redisplay(&runtime);
    }

    if (runtime.mouse_r) {
      point_list_clear(&points);
      bitmap_fill(&(runtime.screen), PALETTE_BLACK);
      runtime_redisplay(&runtime);
    }

  }

  runtime_destroy(&runtime);
  arena_destroy(arena);
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
