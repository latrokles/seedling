#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "base.h"
#include "arena.h"
#include "string8.h"
#include "http.h"
#include "http.c"
#include "yt.h"

int main(int argc, char *argv[]) {
  HttpClient client = http_client_create();
  if (!client.created) { return -1; }

  MemoryArena *arena = arena_create(2000000);
  YoutubeSearchResponse response;

  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    String8 parsed = STRING8(line);

    if (string8_startswith(parsed, STRING8("/help"))) {
      printf("\t- /help.   this help menu.\n");
      printf("\t- /search  search for videos to play.\n");
      printf("\t- /play.   play video with index.\n");
      printf("\t- /quit    quit.\n");
    }

    if (string8_startswith(parsed, STRING8("/search "))) {
      arena_clear(arena);
      String8 query = string8_substringfrom(parsed, 8);
      response = yt_search(client, query, arena);

      printf("Searching for query=%s\n", query.data);
      printf("Total videos found: %zu\n", response.video_count);
      for (usize i = 0; i < response.video_count; i++) {
        // TODO add method to get a specific video by index
        VideoData video = response.videos[i];

        // TODO unpack this better
        printf("%zu: %s\t%s\t%s\n", i, video.uid.data, video.length.data, video.title.data);
      }
    }

    if (string8_startswith(parsed, STRING8("/play "))) {
      String8 selected = string8_substringfrom(parsed, 6);
      usize index = atoi(selected.data);
      VideoData video_to_play = response.videos[index];

      printf("Playing url=%s...\n", video_to_play.url.data);
      String8 video_arg = string8_join(arena, STRING8(""), 3, STRING8("'"), video_to_play.url, STRING8("'"));
      system(string8_concat(STRING8("mpv "), video_arg, arena).data);
    }

    if (string8_startswith(parsed, STRING8("/quit"))) {
      printf("bye!\n");
      break;
    }
  }

  arena_destroy(arena);
  http_client_destroy(&client);
  return 0;
}
