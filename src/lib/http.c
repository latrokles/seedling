#include "http.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Used to store and process the contents of a CURL data transfer.
 * I found it commplicated to pass an arena into this so opted for
 * malloc/realloc/free here and then copy it into an arena allocated
 * String8
 */
typedef struct Chunk {
  char* memory;
  usize size;
} Chunk;

static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp);

HttpClient http_client_create() {
  HttpClient client = { .created = false };

  if ((client.curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "ERROR: Failed to create HttpClient, curl init failed.");
  } else {
    client.created = true;
  }

  return client;
}

void http_client_destroy(HttpClient *client) {
  curl_easy_cleanup(client->curl);
  client->curl = NULL;
  client->created = false;
}

String8 http_response_get_header(HttpResponse *resp, String8 header_name) {
  for (usize i = 0; i < resp->header_count; i++) {
    if (string8_startswith(resp->headers[i], header_name)) {
      return string8_substringfrom(resp->headers[i], header_name.length + 1);
    }
  }
  return (String8){ .length = 0 };
}

HttpResponse http_post(HttpClient client, HttpRequest request, MemoryArena *arena) {
  CURLcode code;

  long http_code = 0L;
  struct curl_slist *headers = NULL;
  Chunk chunk = { .memory = NULL, .size = 0 };

  for (size_t header_idx = 0; header_idx < request.header_count; header_idx++) {
    // TODO: DEBUG log headers
    headers = curl_slist_append(headers, request.headers[header_idx].data);
  }

  curl_easy_setopt(client.curl, CURLOPT_URL, request.uri.data);         // set url
  curl_easy_setopt(client.curl, CURLOPT_HTTPHEADER, headers);           // set headers
  curl_easy_setopt(client.curl, CURLOPT_POSTFIELDS, request.body.data); // set POST method and body
  curl_easy_setopt(client.curl, CURLOPT_WRITEFUNCTION, curl_callback);  // set callback
  curl_easy_setopt(client.curl, CURLOPT_WRITEDATA, &chunk);             // set pointer to response
  curl_easy_setopt(client.curl, CURLOPT_TIMEOUT, 5L);                   // set timeout in seconds
  curl_easy_setopt(client.curl, CURLOPT_FOLLOWLOCATION, 1L);            // follow redirects
  curl_easy_setopt(client.curl, CURLOPT_MAXREDIRS, 1L);                 // max 1 redirect
  curl_easy_setopt(client.curl, CURLOPT_USERAGENT, USER_AGENT);         // set user agent
  // TODO: conditional set CURLOPT_VERBOSE if in debug mode

  code = curl_easy_perform(client.curl);
  if (code != CURLE_OK) {
    fprintf(stderr, "ERROR: Failed to perform network call, url=%s, error=%s.",
            request.uri.data, curl_easy_strerror(code));

    free(chunk.memory);
    curl_slist_free_all(headers);
    curl_easy_reset(client.curl);
    return (HttpResponse){ .status = (usize)http_code };
  }

  String8 body = string8_from_charbuf(arena, chunk.memory, chunk.size);

  free(chunk.memory);
  curl_easy_getinfo(client.curl, CURLINFO_RESPONSE_CODE, &http_code);
  curl_slist_free_all(headers);
  curl_easy_reset(client.curl);

  return (HttpResponse){ .status = (usize) http_code, .body = body };
}

static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb; // calculate buffer size
  Chunk *chunk = (Chunk *)userp;   // cast ptr to body struct ptr

  // reallocate data buffer
  chunk->memory = (char *)realloc(chunk->memory, chunk->size + real_size + 1);

  if (chunk->memory == NULL) {
    fprintf(stderr, "ERROR: Failed to expand buffer to store response body.");
    // CURL will report this as a failure
    return 0;
  }

  // copy response body data
  memcpy(&(chunk->memory[chunk->size]), contents, real_size);
  chunk->size += real_size;
  chunk->memory[chunk->size] = 0;
  return real_size;
}
