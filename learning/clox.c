/* One File LOX Implementation */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -- MEMORY UTILS
#define GROW_CAPACITY(capacity) \
  ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, old_count, new_count)			\
  (type*)reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))

#define FREE_ARRAY(type, pointer, old_count)	\
  reallocate(pointer, sizeof(type) * (old_count), 0)

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  if (new_size == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, new_size);
  if (result == NULL) {
    exit(1);
  }

  return result;
}

// -- TYPES & VALUES

typedef double Value;

typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

void print_value(Value value) {
  printf("%g", value);
}

void init_value_array(ValueArray* array) {
  array->count = 0;
  array->capacity = 0;
  array->values = NULL;
}

void write_value_array(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int old_capacity = array->capacity;
    array->capacity = GROW_CAPACITY(old_capacity);
    array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void free_value_array(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  init_value_array(array);
}

typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

/*
 * Chunks represents sequences of bytecode as a dynamic array of instructions.
 */
typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  ValueArray constants;
} Chunk;


void init_chunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  init_value_array(&chunk->constants);
}

void write_chunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int old_capacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(old_capacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

int add_constant(Chunk* chunk, Value value) {
  write_value_array(&chunk->constants, value);
  return chunk->constants.count - 1;
}

void free_chunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

// -- DEBUG
int disassemble_instruction(Chunk* chunk, int offset);

static int simple_instruction(const char* name, int offset);
static int constant_instruction(const char* name, Chunk* chunk, int offset);

void disassemble_chunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassemble_instruction(chunk, offset);
  }
}

int disassemble_instruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch(instruction) {
  case OP_CONSTANT:
    return constant_instruction("OP_CONSTANT", chunk, offset);
  case OP_RETURN:
    return simple_instruction("OP_RETURN", offset);
  default:
    printf("Unknown OpCode %d\n", instruction);
    return offset + 1;
  }
}

static int simple_instruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  print_value(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}

int main(int argc, const char* argv[]) {
  Chunk chunk;
  init_chunk(&chunk);

  int constant = add_constant(&chunk, 1.2);
  write_chunk(&chunk, OP_CONSTANT, 123);
  write_chunk(&chunk, constant, 123);

  write_chunk(&chunk, OP_RETURN, 123);
  disassemble_chunk(&chunk, "test chunk");
  free_chunk(&chunk);
  return 0;
}
