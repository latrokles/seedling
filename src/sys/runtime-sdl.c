#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "base.h"
#include "draw.h"

typedef enum Key {
  K_UNKNOWN    = 0,
  K_RETURN     = '\r',
  K_ESCAPE     = '\x1B',
  K_BACKSPACE  = '\b',
  K_BACKTICK   = '`',
  K_TAB        = '\t',
  K_LBRACKET   = '[',
  K_RBRACKET   = ']',
  K_BACKSLASH  = '\\',
  K_SEMICOLON  = ';',
  K_QUOTE      = '\'',
  K_COMMA      = ',',
  K_PERIOD     = '.',
  K_SLASH      = '/',
  K_SPACE      = ' ',
  K_1          = '1',
  K_2          = '2',
  K_3          = '3',
  K_4          = '4',
  K_5          = '5',
  K_6          = '6',
  K_7          = '7',
  K_8          = '8',
  K_9          = '9',
  K_0          = '0',
  K_DASH       = '-',
  K_EQUAL      = '=',
  K_A          = 'a',
  K_B          = 'b',
  K_C          = 'c',
  K_D          = 'd',
  K_E          = 'e',
  K_F          = 'f',
  K_G          = 'g',
  K_H          = 'h',
  K_I          = 'i',
  K_J          = 'j',
  K_K          = 'k',
  K_L          = 'l',
  K_M          = 'm',
  K_N          = 'n',
  K_O          = 'o',
  K_P          = 'p',
  K_Q          = 'q',
  K_R          = 'r',
  K_S          = 's',
  K_T          = 't',
  K_U          = 'u',
  K_V          = 'v',
  K_W          = 'w',
  K_X          = 'x',
  K_Y          = 'y',
  K_Z          = 'z',
} Key;

typedef enum FnKey {
  FN_F1,
  FN_F2,
  FN_F3,
  FN_F4,
  FN_F5,
  FN_F6,
  FN_F7,
  FN_F8,
  FN_F9,
  FN_F10,
  FN_F11,
  FN_F12,
  FN_MUTE,
  FN_BRIGHT_DOWN,
  FN_BRIGHT_UP,
  FN_HOME,
  FN_END,
  FN_INSERT,
  FN_PAGE_UP,
  FN_PAGE_DOWN,
  FN_PRINT_SCREEN,
  FN_COUNT,  // count of supported fn keys
} FnKey;

typedef enum ModKey {
  MOD_SCROLL,
  MOD_NUM,
  MOD_CAPS,
  MOD_LCTRL,
  MOD_RCTRL,
  MOD_LALT,
  MOD_RALT,
  MOD_LGUI,
  MOD_RGUI,
  MOD_LSHIFT,
  MOD_RSHIFT,
  MOD_CTRL,
  MOD_ALT,
  MOD_GUI,
  MOD_SHIFT,
  MOD_COUNT,  // count of supported mod keys
} ModKey ;

typedef struct Keyboard {
  bool keys[256];
  bool fn_keys[FN_COUNT];
  bool mod_keys[MOD_COUNT];
} Keyboard;

typedef struct Runtime Runtime;
struct Runtime {
  String8 title;
  Point pos;

  i32 width;
  i32 height;
  u32 zoom;

  Keyboard keyboard;

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
  void (*on_step)(Runtime *);
  void (*on_text_in)(Runtime *, String8);
  void (*on_key_down)(Runtime *);
  void (*on_key_up)(Runtime *);
  void (*on_mouse_down)(Runtime *);
  void (*on_mouse_up)(Runtime *);
  void (*on_mouse_motion)(Runtime *);
};


/* --- prototypes for runtime callbacks --- */
void on_step(Runtime *runtime);
void on_key_down(Runtime *runtime);
void on_key_up(Runtime *runtime);
void on_text_in(Runtime *runtime, String8 s);
void on_mouse_down(Runtime *runtime);
void on_mouse_up(Runtime *runtime);
void on_mouse_motion(Runtime *runtime);

/* --- runtime prototypes --- */
Runtime runtime_create(MemoryArena *arena, String8 title, Point position, i32 width, i32 height, u32 zoom);

void runtime_start(Runtime *runtime);
void runtime_update(Runtime *runtime);
void runtime_stop(Runtime *runtime);

void runtime_redisplay(Runtime *runtime);
void runtime_destroy(Runtime *runtime);

void _run(Runtime *runtime);
void _step(Runtime *runtime);
void _key_down(Runtime *runtime, SDL_Event event);
void _key_up(Runtime *runtime, SDL_Event event);
void _text_in(Runtime *runtime, SDL_Event event);
void _mouse_down(Runtime *runtime, SDL_Event event);
void _mouse_up(Runtime *runtime, SDL_Event event);
void _mouse_pos(Runtime *runtime, SDL_Event event);

Key  __map_key(SDL_Keysym key);
void __print_key_info(String8 type, SDL_Keysym key);

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
  SDL_StartTextInput();
  runtime->is_executing = true;
  runtime_redisplay(runtime);
  _run(runtime);
}

void runtime_stop(Runtime *runtime) {
  runtime->is_executing = false;
  SDL_StopTextInput();
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

  if (runtime->on_step) {
    runtime->on_step(runtime);
  }

  SDL_Event event;
  while (SDL_PollEvent(&event) !=0) {
    switch(event.type) {
    case SDL_QUIT:
      runtime_stop(runtime);
      break;
    case SDL_TEXTINPUT:
      _text_in(runtime, event);
      runtime->needs_redisplay = true;
    case SDL_KEYDOWN:
      _key_down(runtime, event);
      runtime->needs_redisplay = true;
      break;
    case SDL_KEYUP:
      _key_up(runtime, event);
      runtime->needs_redisplay = true;
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

void _text_in(Runtime *runtime, SDL_Event event) {
  // https://wiki.libsdl.org/SDL2/SDL_TextInputEvent
  char *captured = event.text.text;
  u64 captured_len = strlen(event.text.text);

  printf("TextInput: text=%s, length=%lu\n", captured, captured_len);
  String8 s = {captured, captured_len};
  if (runtime->on_text_in) { runtime->on_text_in(runtime, s); }
}

void _key_down(Runtime *runtime, SDL_Event event) {
  __print_key_info(STRING8("Pressed"), event.key.keysym);

  i32 code = (i32)(event.key.keysym.sym);
  if (code >= 0 && code < 256) {
    runtime->keyboard.keys[code] = true;
  }

  // TODO handle fn keys, modifiers, arrows
  if (runtime->on_key_down) { runtime->on_key_down(runtime); }
}

void _key_up(Runtime *runtime, SDL_Event event) {
  __print_key_info(STRING8("Released"), event.key.keysym);

  i32 code = (i32)(event.key.keysym.sym);
  if (code >= 0 && code < 256) {
    runtime->keyboard.keys[code] = false;
  }

  // TODO handle fn keys, modifiers, arrows
  if (runtime->on_key_up) { runtime->on_key_up(runtime); }
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

  if (runtime->on_mouse_down) { runtime->on_mouse_down(runtime); }
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

  if (runtime->on_mouse_up) { runtime->on_mouse_up(runtime); }
}

void _mouse_pos(Runtime *runtime, SDL_Event event) {
  runtime->mouse_prev.x = runtime->mouse_curr.x;
  runtime->mouse_prev.y = runtime->mouse_curr.y;
  runtime->mouse_curr.x = event.motion.x;
  runtime->mouse_curr.y = event.motion.y;

  if (runtime->on_mouse_motion) { runtime->on_mouse_motion(runtime); }
}

void __print_key_info(String8 type, SDL_Keysym key) {
  printf("%s: ", type.data);
  printf("sym=%d, ", key.sym);
  printf("key=%s, ", SDL_GetKeyName(key.sym));

  // https://wiki.libsdl.org/SDL2/SDL_Keymod
  printf("modifiers=[");
  if (key.mod == KMOD_NONE) { printf("N/A"); }

  if (key.mod & KMOD_NUM)    { printf(" NUMLOCK "); }
  if (key.mod & KMOD_CAPS)   { printf(" CAPSLOCK "); }
  if (key.mod & KMOD_SCROLL) { printf(" SCROLLLOCK "); }
  if (key.mod & KMOD_LCTRL)  { printf(" LCTRL "); }
  if (key.mod & KMOD_RCTRL)  { printf(" RCTRL "); }
  if (key.mod & KMOD_LSHIFT) { printf(" LSHIFT "); }
  if (key.mod & KMOD_RSHIFT) { printf(" RSHIFT "); }
  if (key.mod & KMOD_LALT)   { printf(" LALT "); }
  if (key.mod & KMOD_RALT)   { printf(" RALT "); }
  if (key.mod & KMOD_LGUI)   { printf(" LGUI "); }
  if (key.mod & KMOD_RGUI)   { printf(" LGUI "); }
  if (key.mod & KMOD_CTRL)   { printf(" CTRL "); }
  if (key.mod & KMOD_SHIFT)  { printf(" SHIFT "); }
  if (key.mod & KMOD_ALT)    { printf(" ALT "); }
  if (key.mod & KMOD_GUI)    { printf(" GUI "); }
  printf("]\n");
}
