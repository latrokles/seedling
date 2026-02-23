#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "../src/http.h"

// TODO use a proper testing library

int main(void) {
  CURL *curl;

  /* initialize curl handle */
  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "ERROR: Failed to create curl handle");
    return 1;
  }

  HttpResponse resp;
  HttpResponse *resp_ptr = &resp;

  char line[] = "hollow purple 1hr";
  char *url = "https://www.youtube.com/youtubei/v1/search?key=None";
  json_object *body = json_object_from_file("data/youtube-search-request.json");
  json_object_object_add(body, "query", json_object_new_string(line));

  HttpRequest req = {
    .method = "POST",
    .uri = url,
    .body = json_object_to_json_string(body),
    .header_count = 2,
    .headers = { "Accept: application/json", "Content-Type: application/json" }
  };

  int http_code = http_post(curl, req, resp_ptr);
  assert((printf("http_code=%d must be 200\n", http_code), http_code == 200));
  assert(resp.body_size > 0);

  printf("http POST response:\n%s\n", resp.body);

  destroy_http_response(resp_ptr);
  json_object_put(body);
  curl_easy_cleanup(curl);
  return 0;
}
