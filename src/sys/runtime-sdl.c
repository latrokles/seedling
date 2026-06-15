#include <assert.h>
#include <stdio.h>

#include <SDL.h>

#include "base.h"
#include "draw2.h"

typedef struct Runtime Runtime;
struct Runtime {
  String8 title;
  Point pos;

  i32 width;
  i32 height;
  u32 zoom;

  int keys[256];
  int mod;

  Point mouse_curr;
  Point mouse_prev;

  bool mouse_l;
  bool mouse_m;
  bool mouse_r;

  u32 fps;
  bool is_executing;
  bool needs_redisplay;

  Bitmap screen;
  SDL_Window   *window;
  SDL_Renderer *renderer;
  SDL_Texture  *texture;

  void *context;
  void (*on_mouse_down)(Runtime *);
  void (*on_mouse_up)(Runtime *);
  void (*on_mouse_motion)(Runtime *);
};


Runtime runtime_create(MemoryArena *arena, String8 title, Point position, i32 width, i32 height, u32 zoom);

void runtime_start(Runtime *runtime);
void runtime_update(Runtime *runtime);
void runtime_stop(Runtime *runtime);

void runtime_redisplay(Runtime *runtime);
void runtime_destroy(Runtime *runtime);

void _run(Runtime *runtime);
void _step(Runtime *runtime);
void _mouse_down(Runtime *runtime, SDL_Event event);
void _mouse_up(Runtime *runtime, SDL_Event event);
void _mouse_pos(Runtime *runtime, SDL_Event event);

Runtime runtime_create(MemoryArena *arena, String8 title, Point position, i32 width, i32 height, u32 zoom) {
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
    .mouse_l = false,
    .mouse_m = false,
    .mouse_r = false,
    .fps = 60,
    .is_executing = false,
    .needs_redisplay = true,
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
  _run(runtime);
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
  runtime->needs_redisplay = false;
}

void runtime_destroy(Runtime *runtime) {
  // clean up sdl resources
  SDL_DestroyTexture(runtime->texture);
  SDL_DestroyRenderer(runtime->renderer);
  SDL_DestroyWindow(runtime->window);
}

void _run(Runtime *runtime) {
  f64 frame_delay = (f64)1000 / runtime->fps;

  while (runtime->is_executing) {
    u64 frame_start = SDL_GetTicks64();
    _step(runtime);
    u64 frame_end = SDL_GetTicks64();
    u64 frame_time = frame_end - frame_start;
    if (frame_delay > frame_time) {
      if (frame_time > 16) {
	printf("frame took %lu ms.\n", frame_time);
      }
      SDL_Delay(frame_delay - frame_time);
    }
  }
}

void _step(Runtime *runtime) {
  if (runtime->needs_redisplay) {
    runtime_redisplay(runtime);
  }

  SDL_Event event;
  while (SDL_PollEvent(&event) !=0) {
    switch(event.type) {
    case SDL_QUIT:
      runtime_stop(runtime);
      break;
    case SDL_MOUSEBUTTONDOWN:
      _mouse_down(runtime, event);
      runtime->needs_redisplay = true;
      break;
    case SDL_MOUSEBUTTONUP:
      _mouse_up(runtime, event);
      runtime->needs_redisplay = true;
      break;
    case SDL_MOUSEMOTION:
      _mouse_pos(runtime, event);
      runtime->needs_redisplay = true;
      break;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        runtime->needs_redisplay = true;
      }
      break;
    default:
      break;
    }
  }

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

  if (runtime->on_mouse_down) {
    runtime->on_mouse_down(runtime);
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

  if (runtime->on_mouse_up) {
    runtime->on_mouse_up(runtime);
  }
}

void _mouse_pos(Runtime *runtime, SDL_Event event) {
  runtime->mouse_prev.x = runtime->mouse_curr.x;
  runtime->mouse_prev.y = runtime->mouse_curr.y;
  runtime->mouse_curr.x = event.motion.x;
  runtime->mouse_curr.y = event.motion.y;

  if (runtime->on_mouse_motion) {
    runtime->on_mouse_motion(runtime);
  }
}
