#include "unity.h"
#include "unity_fixture.h"

#include "test_str8.c"

TEST_GROUP_RUNNER(Str8Tests) {
  RUN_TEST_CASE(Str8Tests, str8lit_creates_str8_struct);
  RUN_TEST_CASE(Str8Tests, str8from_charbuff_creates_str8_struct_from_char_ptr);
  RUN_TEST_CASE(Str8Tests, str8clone_copies_str8);
}
