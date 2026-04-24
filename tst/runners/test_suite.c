#include "unity_fixture.h"

#include "test_http_runner.c"
#include "test_string8_runner.c"


static void run_unit_tests(void) {
  RUN_TEST_GROUP(String8Tests);
}

static void run_integ_tests(void) {
  RUN_TEST_GROUP(HttpTests);
}

static void run_full_test_suite(void) {
  run_unit_tests();
  run_integ_tests();
}

int main(int argc, const char *argv[]) {
  return UnityMain(argc, argv, run_full_test_suite);
}
