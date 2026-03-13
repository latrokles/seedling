#ifndef __HTTP_H__
#define __HTTP_H__
#include <curl/curl.h>

#define USER_AGENT                                                             \
  "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like " \
  "Gecko) Chrome/70.0.3538.77 Safari/537.36"

/*
  TODO: methods to get a given header value from a request response.
  TODO: methods to create and destroy request and response structs.
*/

// TODO: evaluate defining an HttpHeader struct instead of char *headers[]
// TODO: add support for query params

typedef struct HttpRequest {
  String8 method;
  String8 uri;
  String8 body;
  String8 *headers;
  usize header_count;
} HttpRequest;

typedef struct HttpResponse {
  usize status;
  String8 body;
  String8 *headers;
  usize header_count;
} HttpResponse;


String8 http_response_get_header(HttpResponse *resp, String8 header_name);

HttpResponse http_post(CURL *curl, HttpRequest request, MemoryArena *arena);

#endif
