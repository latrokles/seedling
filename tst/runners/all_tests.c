#include "unity_fixture.h"

#include "test_str8_runner.c"

static void run_all_tests(void) {
  RUN_TEST_GROUP(Str8Tests);
}

int main(int argc, const char *argv[]) {
  return UnityMain(argc, argv, run_all_tests);
}
