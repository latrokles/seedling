#include "unity.h"
#include "unity_fixture.h"

#include "test_http.c"

TEST_GROUP_RUNNER(HttpTests) {
  RUN_TEST_CASE(HttpTests, http_post_makes_successful_post_request);
}
