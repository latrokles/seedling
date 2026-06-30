#include <time.h>

#include "base.h"

typedef struct Status {
  bool is_ok;
  String8 err;
} Status;

typedef struct Editor {
  Buffer *buffer_chain;
  Buffer *current_buffer;
} Editor;

typedef enum MarkType {
  MARK_FIXED,
  MARK_NORMAL,
} MarkType;

typedef struct Mark Mark;
struct Mark {
  Mark *next_mark;
  i32 location;
  MarkType type;
};

typedef struct EditorMode EditorMode;
struct EditorMode {
  EditorMode *next_mode;
  String8 name;
  Status (*add_proc)();
};

typedef struct Buffer Buffer;
struct Buffer {
  Buffer *next_buffer;
  Buffer *prev_buffer;
  String8 name;
  i32 point;
  i32 current_line;
  i32 char_count;
  i32 line_count;
  Mark *mark_list;
  GapBuffer *contents;
  time_t last_sync;
  bool is_modified;
  EditorMode *mode_list;
};
