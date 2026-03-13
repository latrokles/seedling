#include "unity_fixture.h"

#include "test_string8_runner.c"

static void run_all_tests(void) {
  RUN_TEST_GROUP(String8Tests);
}

int main(int argc, const char *argv[]) {
  return UnityMain(argc, argv, run_all_tests);
}
