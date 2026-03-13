#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "base.h"
#include "arena.h"
#include "string8.h"
#include "http.h"
#include "http.c"

#define JSON_MAX_DETPH 100

typedef struct Thumbnail {
  char *url;
  int width;
  int height;
} Thumbnail;

typedef struct Video {
  char *uid;
  char *title;
  char *length;
  Thumbnail *thumbnails;
} Video;

String8 parse_query(int argc, char *argv[]);
void search(HttpClient client, String8 query);
String8 prepare_request_body(String8 query, MemoryArena *arena);
void parse_response_body(String8 body);

int main(int argc, char *argv[]) {
  HttpClient client = http_client_create();
  if (!client.created) { return -1; }

  String8 query = parse_query(argc, argv);
  search(client, query);

  http_client_destroy(&client);
  return 0;
}

String8 parse_query(int argc, char *argv[]) {
  if (argc == 1) {
    return (String8){ .data = argv[0], .length = strlen(argv[0]) };
  }
  return STRING8("hollow purple 1hr");
}

void search(HttpClient client, String8 query) {
  MemoryArena *arena = arena_create(sizeof(char) * 2000000);

  String8 url = STRING8("https://www.youtube.com/youtubei/v1/search?key=None");
  String8 body = prepare_request_body(STRING8("hollow purple 1hr"), arena);
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

  HttpResponse resp = http_post(client, req, arena);
  if (resp.status != 200) {
    fprintf(stderr, "ERROR: Failed to query yt, failed with http response %zu.", resp.status);
    exit(1);
  }

  if (resp.body.length > 0) {
    parse_response_body(resp.body);
  }
  arena_destroy(arena);
}

String8 prepare_request_body(String8 query, MemoryArena *arena) {
  json_object *body = json_object_from_file("data/youtube-search-request.json");
  json_object_object_add(body, "query", json_object_new_string(query.data));

  char *jsonstr = (char *)json_object_to_json_string(body);
  String8 req_body = string8_from_charbuf(jsonstr, strlen(jsonstr), arena);

  json_object_put(body);
  return req_body;
}

void parse_response_body(String8 body) {
  struct json_tokener *tokener = json_tokener_new_ex(JSON_MAX_DETPH);
  enum json_tokener_error json_error = json_tokener_success;

  json_object *jsonbody = json_tokener_parse_ex(tokener, body.data, -1);
  json_error = json_tokener_get_error(tokener);

  if (json_error != json_tokener_success) {
    fprintf(stderr, "ERROR: Failed to parse json string: %u\n", json_error);
  } else {
    printf("%s\n", json_object_to_json_string_ext(jsonbody, JSON_C_TO_STRING_PRETTY));
  }
  json_object_put(jsonbody);
}
