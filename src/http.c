#include "http.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t curl_callback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  size_t real_size = size * nmemb;                // calculate buffer size
  HttpResponse *response = (HttpResponse *)userp; // cast ptr to body struct ptr

  // reallocate data buffer
  response->body =
      (char *)realloc(response->body, response->body_size + real_size + 1);
  if (response->body == NULL) {
    fprintf(stderr, "ERROR: Failed to expand buffer to store response body.");
    exit(71); // no system resources available
  }

  // copy response body data
  memcpy(&(response->body[response->body_size]), contents, real_size);
  response->body_size += real_size;
  response->body[response->body_size] = 0;
  return real_size;
}

int http_post(CURL *curl, HttpRequest request, HttpResponse *response) {
  CURLcode code;

  int http_code = 0;
  struct curl_slist *headers = NULL;

  response->body = (char *)calloc(1, sizeof(response->body));
  if (response->body == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate memory for response body.");
    exit(71); // no system resources available
  }
  response->body_size = 0;

  for (size_t header_idx = 0; header_idx < request.header_count; header_idx++) {
    headers = curl_slist_append(headers, request.headers[header_idx]);
    // TODO: DEBUG log headers
  }

  curl_easy_setopt(curl, CURLOPT_URL, request.uri);    // set url
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); // set headers
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
                   request.body); // set POST method and body
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); // set callback
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                   (void *)response);                // set pointer to response
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);        // set timeout in seconds
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // follow redirects
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);      // max 1 redirect
  curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT); // set user agent
  // TODO: conditional set CURLOPT_VERBOSE if in debug mode

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    fprintf(stderr, "ERROR: Failed to perform network call, url=%s, error=%s.",
            request.uri, curl_easy_strerror(code));

    curl_slist_free_all(headers);
    curl_easy_reset(curl);
    return http_code;
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  curl_slist_free_all(headers);
  curl_easy_reset(curl);

  return http_code;
}
