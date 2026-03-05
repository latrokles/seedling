#include <stdio.h>

#include "unity.h"
#include "unity_fixture.h"

#include "arena.h"
#include "base.h"
#include "str8.h"

TEST_GROUP(Str8Tests);

MemoryArena *arena;

TEST_SETUP(Str8Tests) { arena = arena_create(sizeof(char) * 100); }

TEST_TEAR_DOWN(Str8Tests) { arena_destroy(arena); }

TEST(Str8Tests, str8lit_creates_str8_struct) {
  Str8 s = STR8LIT("hello world");

  TEST_ASSERT_EQUAL_STRING("hello world", s.data);
  TEST_ASSERT_EQUAL(11, s.size);
}

TEST(Str8Tests, str8from_charbuff_creates_str8_struct_from_char_ptr) {
  char *buff = "foo";
  Str8 s = str8from_charbuff(buff, 3);

  TEST_ASSERT_EQUAL_STRING("foo", s.data);
  TEST_ASSERT_EQUAL(3, s.size);
}

TEST(Str8Tests, str8clone_copies_str8) {
  Str8 s = STR8LIT("hello world");
  Str8 copy = str8clone(arena, s);

  TEST_ASSERT_EQUAL_STRING(s.data, copy.data);
}
