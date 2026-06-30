#include <stdlib.h>
#include <unistd.h>


#include "base.h"
#include "draw.h"
#include "font.h"
#include "runtime-sdl.c"

typedef struct Theme {
  Color background;
  Color usr_text_color;
  Color sys_text_color;
  Color err_text_color;

  String8 usr_font_path;
  String8 sys_font_path;
  String8 err_font_path;

  i32 usr_font_size;
  i32 sys_font_size;
  i32 err_font_size;
} Theme;

typedef struct DisplayManager {
  Theme theme;
  i32 display_w;
  i32 display_h;

  Font *usr_font;
  Font *sys_font;
  Font *err_font;

  Bitmap *line_painter;
} DisplayManager;

typedef struct Buffer {
  char **lines;
  i32 length;
  i32 capacity;
  Point position;
  bool has_changed;
} Buffer;

Buffer *buffer_create();
Buffer *buffer_create_from_string(char *s, i32 len);
void buffer_add_line(Buffer *b, char *line);

typedef struct LineEditor {
  Buffer *buffer;
} LineEditor;

typedef struct Fooled {
  DisplayManager display;
  LineEditor editor;
} Fooled;

String8 __read_file(String8 filepath);
Fooled __fooled_create(MemoryArena *arena, i32 width, i32 height);

void on_step(Runtime *runtime) {
}

void on_text_in(Runtime *runtime, String8 s) {
}

void on_key_down(Runtime *runtime) {
}

Fooled __fooled_create(MemoryArena *arena, i32 width, i32 height) {
  Theme theme = {
    .background = 0x22222200,
    .usr_text_color = 0xffffff00,
    .sys_text_color = 0xaffec700,
    .err_text_color = 0xffb54500,
    .usr_font_path = STRING8("fonts/ttf/JetBrainsMonoNL-Thin.ttf"),
    .sys_font_path = STRING8("fonts/ttf/JetBrainsMonoNL-Italic.ttf"),
    .err_font_path = STRING8("fonts/ttf/JetBrainsMonoNL-SemiBold.ttf"),
    .usr_font_size = 16,
    .sys_font_size = 14,
    .err_font_size = 14,
  };

  char *home_env_var = getenv("HOME");
  String8 home_path = string8_from_charbuf(arena, home_env_var, strlen(home_env_var));
  Font usr_font = font_create(arena,
			      string8_join(arena, STRING8("/"), 2, home_path, theme.usr_font_path),
                              theme.usr_font_size,
			      theme.usr_font_size);
  Font sys_font = font_create(arena,
			      string8_join(arena, STRING8("/"), 2, home_path, theme.sys_font_path),
                              theme.sys_font_size,
			      theme.sys_font_size);
  Font err_font = font_create(arena,
			      string8_join(arena, STRING8("/"), 2, home_path, theme.err_font_path),
                              theme.err_font_size,
			      theme.err_font_size);
  Bitmap pen = bitmap_create(arena, 2, 2);
  bitmap_fill(&pen, PALETTE_BLUE);

  DisplayManager display = {
    .theme = theme,
    .display_w = width,
    .display_h = height,
    .usr_font = &usr_font,
    .sys_font = &sys_font,
    .err_font = &err_font,
    .line_painter = &pen,
  };

  LineEditor editor;
  return (Fooled){
    .display = display,
    .editor = editor,
  };
}

int main(int argc, char *argv[]) {
  i32 width = 16 * 80;
  i32 height = 16 * 30;

  MemoryArena *arena = arena_create(20 * MB);
  Runtime r = runtime_create(arena,
			     STRING8("fooled - a text editor"),
			     (Point){-1, -1},
			     width,
			     height,
			     1);
  Fooled program_state = __fooled_create(arena, width, height);
  r.context = (void *)&program_state;
  r.on_step = on_step;
  r.on_text_in = on_text_in;
  r.on_key_down = on_key_down;

  runtime_start(&r);
  runtime_destroy(&r);
  arena_destroy(arena);
}
