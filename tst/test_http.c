#include <assert.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/base.h"
#include "../src/arena.h"
#include "../src/string8.h"
#include "../src/http.h"
#include "../src/http.c"

// TODO use a proper testing library

String8 prepare_request_body(String8 query, MemoryArena *arena) {
  json_object *body = json_object_from_file("data/youtube-search-request.json");
  json_object_object_add(body, "query", json_object_new_string(query.data));

  char *jsonstr = (char *)json_object_to_json_string(body);
  String8 req_body = string8_from_charbuf(jsonstr, strlen(jsonstr), arena);

  printf("json_body=%s\n", (char *)json_object_to_json_string(body));
  printf("json_body_length=%llu\n", (u64)strlen(jsonstr));
  printf("req_body=%s\n", req_body.data);

  json_object_put(body);
  return req_body;
}

int main(void) {
  HttpClient http = http_client_create();
  if (!http.created) {
    exit(1);
  }

  MemoryArena *http_arena = arena_create(sizeof(char) * 2000000);
  String8 url = STRING8("https://www.youtube.com/youtubei/v1/search?key=None");
  String8 body = prepare_request_body(STRING8("hollow purple 1hr"), http_arena);
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


  HttpResponse resp = http_post(http, req, http_arena);
  assert((printf("http_code=%zu must be 200\n", resp.status), resp.status == 200));
  assert(resp.body.length > 0);

  printf("http POST response:\n%s\n", resp.body.data);

  arena_destroy(http_arena);
  http_client_destroy(&http);
  return 0;
}
