#include <assert.h>
#include <stdio.h>

#include "../src/base.h"
#include "../src/str8.h"

void test_str8(void);
void test_str8arr(void);

int main(void) {
  test_str8();
}

void test_str8(void) {
  Str8 *s = &(str8lit("hello world"));
  printf("s->data=%s\n", s->data);
  printf("s->size=%llu\n", s->size);
}

void test_str8arr(void) {
}
