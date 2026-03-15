#include <stdio.h>

#include "unity.h"
#include "unity_fixture.h"

#include "base.h"
#include "arena.h"
#include "string8.h"

TEST_GROUP(String8Tests);

TEST_SETUP(String8Tests) {}

TEST_TEAR_DOWN(String8Tests) {}

TEST(String8Tests, string8_creates_string8_struct) {
  String8 s = STRING8("hello world");

  TEST_ASSERT_EQUAL_STRING("hello world", s.data);
  TEST_ASSERT_EQUAL(11, s.length);
}

TEST(String8Tests, string8_from_charbuf_creates_an_arena_allocated_string8_from_char_buffer) {
  MemoryArena *arena = arena_create(sizeof(char) * 100);

  String8 s = string8_from_charbuf("foo", 3, arena);

  TEST_ASSERT_EQUAL_STRING(s.data, "foo");

  arena_destroy(arena);
}

TEST(String8Tests, string8_clone_copies_string8) {
  MemoryArena *arena = arena_create(sizeof(char) * 100);

  String8 s = STRING8("hello world");
  String8 copy = string8_clone(s, arena);

  TEST_ASSERT_EQUAL_STRING(s.data, copy.data);

  arena_destroy(arena);
}

TEST(String8Tests, string8_concat_returns_a_new_string_joining_lhs_and_rhs) {
  MemoryArena *arena = arena_create(sizeof(char) * 100);

  String8 lhs = STRING8("foo");
  String8 rhs = STRING8("bar");

  TEST_ASSERT_EQUAL_STRING("foobar", string8_concat(lhs, rhs, arena).data);

  arena_destroy(arena);
}

TEST(String8Tests, string8_join_returns_value_if_only_one_value_is_passed) {
  MemoryArena *arena = arena_create(sizeof(char) * 100);

  String8 sep = STRING8(", ");
  String8 joined = string8_join(arena, sep, 1, STRING8("foo"));
  TEST_ASSERT_EQUAL_STRING("foo", joined.data);

  arena_destroy(arena);
}

TEST(String8Tests, string8_join_returns_the_merge_of_its_arguments_with_separator) {
  MemoryArena *arena = arena_create(sizeof(char) * 100);
  String8 sep = STRING8(", ");
  String8 joined = string8_join(arena, sep, 3, STRING8("foo"), STRING8("bar"), STRING8("baz"));

  TEST_ASSERT_EQUAL_STRING("foo, bar, baz", joined.data);
  arena_destroy(arena);
}

TEST(String8Tests, string8_substringfrom_returns_slice_from_start_index_to_end_of_s) {
  String8 substring = string8_substringfrom(STRING8("foobar"), 3);
  TEST_ASSERT_TRUE(string8_equals(substring, STRING8("bar")));
}

TEST(String8Tests, string8_substringfrom_returns_empty_string_when_start_index_is_out_of_bounds) {
  String8 substring = string8_substringfrom(STRING8("bar"), 3);
  TEST_ASSERT_EQUAL(0, substring.length);
}

TEST(String8Tests, string8_get_returns_char_at_index) {
  String8 s = STRING8("hello world");

  TEST_ASSERT_EQUAL('h', string8_get(s, 0));
  TEST_ASSERT_EQUAL('d', string8_get(s, s.length - 1));
}

TEST(String8Tests, string8_compare_returns_negative_value_when_lhs_less_than_rhs) {
  TEST_ASSERT_TRUE(string8_compare(STRING8("bar"), STRING8("baz")) < 0);
}

TEST(String8Tests, string8_compare_returns_positive_value_when_lhs_greater_than_rhs) {
  TEST_ASSERT_TRUE(string8_compare(STRING8("baz"), STRING8("bar")) > 0);
}

TEST(String8Tests, string8_compare_returns_zero_when_lhs_less_equals_rhs) {
  TEST_ASSERT_TRUE(string8_compare(STRING8("baz"), STRING8("baz")) == 0);
}

TEST(String8Tests, string8_equals_returns_true_when_lhs_and_rhs_have_equal_values) {
  TEST_ASSERT_TRUE(string8_equals(STRING8("baz"), STRING8("baz")));
}

TEST(String8Tests, string8_equals_returns_false_when_lhs_and_rhs_have_different_length) {
  TEST_ASSERT_FALSE(string8_equals(STRING8("bar"), STRING8("ba")));
}

TEST(String8Tests, string8_equals_returns_false_when_lhs_and_rhs_dont_have_equal_values) {
  TEST_ASSERT_FALSE(string8_equals(STRING8("bar"), STRING8("baz")));
}

TEST(String8Tests, string8_startswith_returns_true_if_prefix_is_empty) {
  TEST_ASSERT_TRUE(string8_startswith(STRING8("foo"), STRING8("")));
}

TEST(String8Tests, string8_startswith_returns_true_if_s_starts_with_prefix) {
  TEST_ASSERT_TRUE(string8_startswith(STRING8("foobar"), STRING8("foo")));
}

TEST(String8Tests, string8_startswith_returns_false_if_prefix_is_longer_than_s) {
  TEST_ASSERT_FALSE(string8_startswith(STRING8("foo"), STRING8("foobar")));
}

TEST(String8Tests, string8_startswith_returns_false_if_s_does_not_start_with_prefix) {
  TEST_ASSERT_FALSE(string8_startswith(STRING8("foobar"), STRING8("oo")));
}

TEST(String8Tests, string8_endswith_returns_true_if_suffix_is_empty) {
  TEST_ASSERT_TRUE(string8_endswith(STRING8("foo"), STRING8("")));
}

TEST(String8Tests, string8_endswith_returns_true_if_s_ends_with_suffix) {
  TEST_ASSERT_TRUE(string8_endswith(STRING8("foobar"), STRING8("bar")));
}

TEST(String8Tests, string8_endswith_returns_false_if_suffix_is_longer_than_s) {
  TEST_ASSERT_FALSE(string8_endswith(STRING8("bar"), STRING8("bartoz")));
}

TEST(String8Tests, string8_endswith_returns_false_if_s_does_not_end_with_suffix) {
  TEST_ASSERT_FALSE(string8_endswith(STRING8("foobar"), STRING8("ba")));
}
