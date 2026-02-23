#include <curl/curl.h>

#define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36"

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

typedef struct HTTP_Body {
  char *data;
  size_t size;
} HTTP_Body;


int http_post(CURL *curl, char *url, char *http_headers[], size_t header_count, const char *req_body, HTTP_Body *resp_body);
