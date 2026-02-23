#include <stdio.h>


typedef struct HttpHeader {
  char *name;
  char *value;
} HttpHeader;

void print_headers(HttpHeader headers[], size_t count) {

  for (size_t i = 0; i < count; i++) {
    printf("header.name=%s\theader.value=%s\n", headers[i].name, headers[i].value);
  }
}


int main(void) {
  HttpHeader h1 = { .name = "Accept", .value = "application/json" };
  HttpHeader h2 = { .name = "Content-Type", .value = "application/json" };

  HttpHeader headers[] = { h1, h2 };
  size_t count = sizeof(headers) / sizeof(HttpHeader);
  printf("count=%zu\n", count);
  print_headers(headers, count);
  return 0;
}
	
