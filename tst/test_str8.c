#include <assert.h>
#include <stdio.h>

#include "unity.h"
#include "unity_fixture.h"

#include "base.h"
#include "str8.h"

TEST_GROUP(Str8Tests);

TEST_SETUP(Str8Tests) {
}

TEST_TEAR_DOWN(Str8Tests) {
}

TEST(Str8Tests, str8lit_creates_str8_struct) {
  Str8 s = str8lit("hello world");

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
  Str8 s = str8lit("hello world");
  Str8 copy = str8clone(s);

  TEST_ASSERT_EQUAL_STRING(s.data, copy.data);
}

TEST(Str8Tests, str8free_frees_underlying_string_memory_and_sets_size_to_zero) {
  Str8 s = str8from_charbuff("foobar", 6);
  str8free(&s);

  TEST_ASSERT_EQUAL(NULL, s.data);
  TEST_ASSERT_EQUAL(0, s.size);
}
