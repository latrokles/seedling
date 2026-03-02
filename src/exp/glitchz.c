#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>

/*
 * compile with:
 * cc glitchy.c sdl2-config --cflags --libs
 */

typedef uint8_t u8;
typedef uint32_t Color;

// map cartesian coordinates (x, y) in a 2D plane of width (w) to an index in a 1D array/buffer
#define PIXEL_INDEX(x, y, w) (y * w) + x
#define RGBA(r, g, b, a) (Color)(((u8) (r) << 24) | ((u8) (g) << 16) | ((u8) (b) << 8) | (u8) (a))


typedef struct VM {
  Color *screen;
  int screen_width;
  int screen_height;
} VM;

#define WIDTH 300
#define HEIGHT 600

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;


// Set up the runtime
void init_runtime(VM *vm) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to initialize SDL, error=%s\n!", SDL_GetError());
    exit(74);
  }

  int width = vm->screen_width;
  int height = vm->screen_height;

  window   = SDL_CreateWindow("GLITHZZ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, 0);
  texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width, height);

  SDL_SetWindowMinimumSize(window, width, height);
  SDL_RenderSetLogicalSize(renderer, width, height);
  SDL_RenderSetIntegerScale(renderer, 1);

  // SDL_SetWindowBordered(window, false);
}

void stop_runtime(void) {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  exit(0);
}

void redisplay(VM *vm) {
  SDL_UpdateTexture(texture, NULL, vm->screen, vm->screen_width * sizeof(Color));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void draw_pixels(VM* vm) {
  for (int i = 0;  i < (vm->screen_width * vm->screen_height); i++) {
    int x = rand() % vm->screen_width;
    int y = rand() % vm->screen_height;
    int pixel = PIXEL_INDEX(x, y, vm->screen_width);

    Color c = RGBA((rand() % 255), (rand() % 255), (rand() % 255), (rand() % 255));
    vm->screen[pixel] = c;
  }
}

void run(VM *vm, int fps) {
  SDL_Event event;
  int next_tick = 0;
  int running = true;

  redisplay(vm);

  while (running) {
    int tick = SDL_GetTicks();

    if (tick < next_tick) {
      SDL_Delay(next_tick - tick);
    }
    next_tick = tick + (1000 / fps);

    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_QUIT:
	running = false;
	break;
      }
    }
    draw_pixels(vm);
    redisplay(vm);
  }
}


VM* create_vm(int screen_width, int screen_height) {
  VM *vm = (VM*)malloc(sizeof(vm));
  if (vm == NULL) {
    fprintf(stderr, "Unable to allocate memory for VM\n");
    exit(74);
  }

  vm->screen = (Color*)malloc(screen_width * screen_height * sizeof(Color));
  if (vm->screen == NULL) {
    fprintf(stderr, "Unable to allocate memory for screen\n");
    exit(74);
  }

  for (int i = 0;  i < (screen_width * screen_height); i++) {
    vm->screen[i] = 0xeeee9eff;
  }
  vm->screen_width  = screen_width;
  vm->screen_height = screen_height;
  return vm;
}

void destroy_vm(VM *vm) {
  free(vm->screen);
  free(vm);
}

int main(void) {
  VM *vm = create_vm(WIDTH, HEIGHT);
  init_runtime(vm);
  run(vm, 30);
  destroy_vm(vm);
  stop_runtime();
}
