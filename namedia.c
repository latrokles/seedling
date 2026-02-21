#include <stdio.h>
#include <string.h>

#include <json-c/json.h>
#include <curl/curl.h>

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

typedef struct Response {
  char *payload;
  size_t size;
} Response;

size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;            // calculate buffer size
  Response *p = (Response *) userp;   // cast pointer to the response struct

  // reallocate payload buffer
  p->payload = (char *) realloc(p->payload, p->size + real_size + 1);
  if (p->payload == NULL) {
    fprintf(stderr, "ERROR: Failed to expand buffer to store response payload");
    free(p->payload);
    return -1;
  }

  // copy data
  memcpy(&(p->payload[p->size]), contents, real_size);
  p->size += real_size;
  p->payload[p->size] = 0;
  return real_size;
}

CURLcode execute_query(CURL *curl, char *url, Response *response_ptr) {
  CURLcode result_code;

  response_ptr->payload = (char *) calloc(1, sizeof(response_ptr->payload));
  if (response_ptr->payload == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate response payload");
    return CURLE_FAILED_INIT;
  }

  response_ptr->size = 0;

  curl_easy_setopt(curl, CURLOPT_URL, url);                          // set url
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);      // set callback function
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) response_ptr);  // set the pointer to response struct
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);                        // set timeout (in seconds)
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);                 // follow redirects
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);                      // max allowed redirects
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);                         // verbose true
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");

  result_code = curl_easy_perform(curl);
  return result_code;
}

int search(CURL *curl, char query[]) {
  printf("Searching for query=%s\n", query);
  char *url = "https://www.youtube.com/youtubei/v1/search?key=None";

  CURLcode result_code;
  Response response;
  Response *response_ptr = &response;
  struct curl_slist *headers = NULL;

  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");

  json_object *request_json = json_object_from_file("assets/yt-query.json");
  json_object_object_add(request_json, "query", json_object_new_string(query));
  printf("Json request:\n\n%s\n", json_object_to_json_string_ext(request_json, JSON_C_TO_STRING_PRETTY));

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(request_json));

  result_code = execute_query(curl, url, response_ptr);


  curl_slist_free_all(headers);   // free headers
  json_object_put(request_json);  // free request json

  if (result_code != CURLE_OK || response_ptr->size < 1) {
    fprintf(stderr,
	    "ERROR: Failed to fetch url (%s) - curl failed with: %s",
	    url,
	    curl_easy_strerror(result_code));
    return 2;
  }

  struct json_tokener *tokener = json_tokener_new_ex(JSON_MAX_DETPH);
  json_object *response_json = NULL;
  enum json_tokener_error jerr = json_tokener_success;

  if (response_ptr->payload != NULL) {
    printf("Response Payload: \n%s\n", response_ptr->payload);
    response_json = json_tokener_parse_ex(tokener, response_ptr->payload, -1);
    jerr = json_tokener_get_error(tokener);
    // response_json = json_tokener_parse_verbose(response_ptr->payload, &jerr);
    free(response_ptr->payload);
    json_tokener_free(tokener);
  } else {
    fprintf(stderr, "ERROR: Failed to populate payload");
    free(response_ptr->payload);
    json_tokener_free(tokener);
    return 3;
  }

  if (jerr != json_tokener_success) {
    fprintf(stderr, "ERROR: Failed to parse json string: %u\n", jerr);
    json_object_put(response_json);
    printf("RETURNING????\n");
    return 4;
  }

  printf("Parsed JSON: \n%s\n", json_object_to_json_string_ext(response_json, JSON_C_TO_STRING_PRETTY));
  json_object_put(response_json);
  return 0;
}

int main(void) {
  CURL *curl;

  /* initialize curl handle */
  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "ERROR: Failed to create curl handle");
    return 1;
  }

  char line[] = "hollow purple 1hr";
  search(curl, line);

  curl_easy_cleanup(curl);
  return 0;
}
