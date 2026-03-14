#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "base.h"
#include "arena.h"
#include "string8.h"
#include "http.h"
#include "http.c"
#include "yt.h"


String8 parse_query(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  HttpClient client = http_client_create();
  if (!client.created) { return -1; }

  MemoryArena *arena = arena_create(sizeof(char) * 2000000);

  String8 query = parse_query(argc, argv);
  YoutubeSearchResponse response = yt_search(client, query, arena);

  printf("Total videos found: %zu\n", response.video_count);

  for (usize i = 0; i < response.video_count; i++) {
    // TODO add method to get a specific video by index
    VideoData video = response.videos[i];

    // TODO unpack this better
    printf("index=%zu: %s\t%s\t%s\n", i, video.uid.data, video.length.data, video.title.data);
  }

  arena_destroy(arena);
  http_client_destroy(&client);
  return 0;
}

String8 parse_query(int argc, char *argv[]) {
  if (argc == 2) {
    return (String8){ .data = argv[1], .length = strlen(argv[1]) };
  }
  return STRING8("hollow purple 1hr");
}
