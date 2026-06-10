#include <assert.h>
#include <stdio.h>

#include <SDL.h>

#include "draw.h"

typedef struct Runtime {
  String8 title;
  Point pos;

  i64 width;
  i64 height;
  u32 zoom;

  int keys[256];
  int mod;

  i64 mouse_x;
  i64 mouse_y;
  i64 mouse_px;
  i64 mouse_py;
  bool mouse_l;
  bool mouse_m;
  bool mouse_r;

  u32 fps;
  u64 next_tick;
  bool is_executing;
  bool is_updated;

  Bitmap screen;
  SDL_Window   *window;
  SDL_Renderer *renderer;
  SDL_Texture  *texture;
} Runtime;


Runtime runtime_create(MemoryArena *arena, String8 title, Point position, i64 width, i64 height, u32 zoom);

void runtime_start(Runtime *runtime);
void runtime_update(Runtime *runtime);
void runtime_stop(Runtime *runtime);

void runtime_redisplay(Runtime *runtime);
void runtime_destroy(Runtime *runtime);

void _mouse_down(Runtime *runtime, SDL_Event event);
void _mouse_up(Runtime *runtime, SDL_Event event);
void _mouse_pos(Runtime *runtime, SDL_Event event);

Runtime runtime_create(MemoryArena *arena, String8 title, Point position, i64 width, i64 height, u32 zoom) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to initialize SDL backend, error=%s\n", SDL_GetError());
    assert(false);
  }

  position.x = position.x < 0 ? SDL_WINDOWPOS_UNDEFINED : position.x;
  position.y = position.y < 0 ? SDL_WINDOWPOS_UNDEFINED : position.y;

  SDL_Window *window = SDL_CreateWindow(title.data,
				        position.x,
				        position.y,
				        width * zoom,
				        height * zoom,
				        SDL_WINDOW_SHOWN);
  assert(window);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  assert(renderer);

  SDL_Texture *texture = SDL_CreateTexture(renderer,
					   SDL_PIXELFORMAT_RGBA8888,
					   SDL_TEXTUREACCESS_STATIC,
					   width,
					   height);
  assert(texture);

  Bitmap screen = bitmap_create(arena, width, height);

  Runtime sdl = {
    .title = title,
    .pos = position,
    .width = width,
    .height = height,
    .zoom = zoom,
    .mod = 0,
    .mouse_x = 0,
    .mouse_y = 0,
    .mouse_px = 0,
    .mouse_py = 0,
    .mouse_l = false,
    .mouse_m = false,
    .mouse_r = false,
    .fps = 30L,
    .next_tick = 0L,
    .is_executing = false,
    .is_updated = false,
    .screen = screen,
    .window = window,
    .renderer = renderer,
    .texture = texture,
  };
  return sdl;
}

void runtime_start(Runtime *runtime) {
  runtime->is_executing = true;
  runtime_redisplay(runtime);
}

void runtime_update(Runtime *runtime) {
  u64 tick = SDL_GetTicks64();
  if (tick < runtime->next_tick){
    SDL_Delay(runtime->next_tick - tick);
  }
  u64 millis_per_frame = (u64)(1000.0 / runtime->fps);
  runtime->next_tick = tick + millis_per_frame;

  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    switch(event.type) {
    case SDL_QUIT:
      runtime_stop(runtime);
      break;
    case SDL_MOUSEBUTTONDOWN:
      _mouse_down(runtime, event);
      break;
    case SDL_MOUSEBUTTONUP:
      _mouse_up(runtime, event);
      break;
    case SDL_MOUSEMOTION:
      _mouse_pos(runtime, event);
      break;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
	runtime_redisplay(runtime);
      }
      break;
    default:
      break;
    }
  }
}

void runtime_stop(Runtime *runtime) {
  runtime->is_executing = false;
}

void runtime_redisplay(Runtime *runtime) {
  SDL_UpdateTexture(runtime->texture,
		    NULL,
		    runtime->screen.pixels,
		    runtime->width * sizeof(Color));

  SDL_RenderClear(runtime->renderer);
  SDL_RenderCopy(runtime->renderer, runtime->texture, NULL, NULL);
  SDL_RenderPresent(runtime->renderer);
}

void runtime_destroy(Runtime *runtime) {
  // clean up sdl resources
  SDL_DestroyTexture(runtime->texture);
  SDL_DestroyRenderer(runtime->renderer);
  SDL_DestroyWindow(runtime->window);
}

void _mouse_down(Runtime *runtime, SDL_Event event) {
  switch(event.button.button) {
  case SDL_BUTTON_LEFT:
    runtime->mouse_l = true;
    break;
  case SDL_BUTTON_MIDDLE:
    runtime->mouse_m = true;
    break;
  case SDL_BUTTON_RIGHT:
    runtime->mouse_r = true;
    break;
  default:
    break;
  }
}

void _mouse_up(Runtime *runtime, SDL_Event event) {
  switch(event.button.button) {
  case SDL_BUTTON_LEFT:
    runtime->mouse_l = false;
    break;
  case SDL_BUTTON_MIDDLE:
    runtime->mouse_m = false;
    break;
  case SDL_BUTTON_RIGHT:
    runtime->mouse_r = false;
    break;
  default:
    break;
  }
}

void _mouse_pos(Runtime *runtime, SDL_Event event) {
  runtime->mouse_px = runtime->mouse_x;
  runtime->mouse_py = runtime->mouse_y;
  runtime->mouse_x = event.motion.x;
  runtime->mouse_y = event.motion.y;
}
