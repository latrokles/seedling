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

  HTTP_Body resp;
  HTTP_Body *resp_ptr = &resp;

  char line[] = "hollow purple 1hr";
  char *url = "https://www.youtube.com/youtubei/v1/search?key=None";
  char *headers[] = { "Accept: application/json", "Content-Type: application/json" };
  json_object *req = json_object_from_file("data/youtube-search-request.json");

  int http_code = http_post(curl, url, headers, 2, json_object_to_json_string(req), resp_ptr);
  assert((printf("http_code=%d must be 200\n", http_code), http_code == 200));
  assert(resp.size > 0);

  printf("http POST response:\n%s\n", resp.data);

  free(resp_ptr->data);
  json_object_put(req);
  curl_easy_cleanup(curl);
  return 0;
}
