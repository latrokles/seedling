#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json-c/json.h>

#include "unity.h"
#include "unity_fixture.h"

#include "base.h"
#include "arena.h"
#include "string8.h"
#include "http.h"
#include "http.c"

String8 __prepare_post_request_body(String8 query, MemoryArena *arena);

HttpClient http;
MemoryArena *arena;

TEST_GROUP(HttpTests);

TEST_SETUP(HttpTests) {
  http = http_client_create();
  if (!http.created) {
    exit(1);
  }

  arena = arena_create(sizeof(char) * 2000000);
}

TEST_TEAR_DOWN(HttpTests) {
  arena_destroy(arena);
  http_client_destroy(&http);
}

TEST(HttpTests, http_post_makes_successful_post_request) {
  String8 url = STRING8("https://www.youtube.com/youtubei/v1/search?key=None");
  String8 body = __prepare_post_request_body(STRING8("hollow purple 1hr"), arena);
  String8 headers[2] = {
    STRING8("Accept: application/json"),
    STRING8("Content-Type: application/json")
  };

  HttpRequest req = {
    .method = "POST",
    .uri = url,
    .body = body,
    .headers = headers,
    .header_count = 2
  };


  HttpResponse resp = http_post(http, req, arena);
  TEST_ASSERT_EQUAL(200, resp.status);
  TEST_ASSERT_TRUE(resp.body.length);
}

String8 __prepare_post_request_body(String8 query, MemoryArena *arena) {
  json_object *body = json_object_from_file("data/youtube-search-request.json");
  json_object_object_add(body, "query", json_object_new_string(query.data));

  char *jsonstr = (char *)json_object_to_json_string(body);
  String8 req_body = string8_from_charbuf(arena, jsonstr, strlen(jsonstr));

  json_object_put(body);
  return req_body;
}
