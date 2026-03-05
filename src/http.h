#include <curl/curl.h>

#define USER_AGENT                                                             \
  "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like " \
  "Gecko) Chrome/70.0.3538.77 Safari/537.36"

/*
  TODO: define HTTP_Request and HTTP_Response structs

  HTTP Request:
  Method: char*,
  URI: char*
  Headers: char* [], or maybe HTTP_Header[]
  Body: char*


  HTTP Response:
  Status: int
  Headers: char* [], or maybe HTTP_Header[]
  Body: char*

  TODO: methods to get a given header value from a request response.
  TODO: methods to create and destroy request and response structs.
*/

// TODO: evaluate defining an HttpHeader struct instead of char *headers[]
// TODO: add support for query params
typedef struct HttpRequest {
  const char *method;
  const char *uri;
  const char *body;
  size_t header_count;
  char *headers[];
} HttpRequest;

typedef struct HttpResponse {
  int status;
  char *body;
  size_t body_size;
  size_t header_count;
  char *headers[];
} HttpResponse;

typedef struct HTTP_Body {
  char *data;
  size_t size;
} HTTP_Body;

void destroy_http_response(HttpResponse *response) { free(response->body); }

int http_post(CURL *curl, HttpRequest request, HttpResponse *response);
