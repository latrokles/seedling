#ifndef __YT_H__
#define __YT_H__

#include <json-c/json.h>

#define JSON_MAX_DETPH 100
#define YT_SEARCH_URL STRING8("https://www.youtube.com/youtubei/v1/search?key=None")
#define YT_WATCH_URL  STRING8("https://www.youtube.com/watch?v=")

enum YoutubeErrorCode {
  YT_SEARCH_OK,
  YT_SEARCH_NONE,
  YT_SEARCH_ERROR,
  YT_SEARCH_PARSE_ERROR
} YoutubeErrorCode;


typedef struct VideoData {
  String8 uid;
  String8 title;
  String8 length;
  String8 url;
} VideoData;

typedef struct YoutubeSearchResponse {
  enum YoutubeErrorCode code;
  VideoData *videos;
  usize video_count;
} YoutubeSearchResponse;

YoutubeSearchResponse yt_search(HttpClient client, String8 query, MemoryArena *arena);

static String8 prepare_request_body(String8 query, MemoryArena *arena);
static YoutubeSearchResponse parse_response(String8 body, MemoryArena *arena);
static VideoData parse_video_data(json_object *json, MemoryArena *arena);
static String8 get_video_id(json_object *video_renderer_json, MemoryArena *arena);
static String8 get_title(json_object *video_renderer_json, MemoryArena *arena);
static String8 get_length(json_object *video_renderer_json, MemoryArena *arena);

YoutubeSearchResponse yt_search(HttpClient client, String8 query, MemoryArena *arena) {
  // TODO consider using a scratch arena for the http call and then allocate the parsed
  //      videos from the `arena` passed in

  String8 headers[2] = {
    STRING8("Accept: application/json"),
    STRING8("Content-Type: application/json")
  };

  MemoryArena *scratch_arena = arena_create(20000000);
  String8 body = prepare_request_body(query, scratch_arena);
  HttpRequest req = {
    .method = "POST",
    .uri = YT_SEARCH_URL,
    .body = body,
    .headers = headers,
    .header_count = 2
  };

  HttpResponse resp = http_post(client, req, scratch_arena);
  if (resp.status != 200) {
    fprintf(stderr, "ERROR: Failed to query yt, failed with http response %zu.", resp.status);
    return (YoutubeSearchResponse){ .code = YT_SEARCH_ERROR, .videos = NULL };
  }

  YoutubeSearchResponse response = parse_response(resp.body, arena);
  arena_destroy(scratch_arena);
  return response;
}

static String8 prepare_request_body(String8 query, MemoryArena *arena) {
  json_object *body = json_object_from_file("data/youtube-search-request.json");
  json_object_object_add(body, "query", json_object_new_string(query.data));

  char *jsonstr = (char *)json_object_to_json_string(body);
  String8 req_body = string8_from_charbuf(jsonstr, strlen(jsonstr), arena);

  json_object_put(body);
  return req_body;
}

/*
  The list of results to parse looks like:
  {
    "estimatedResults": "1721878",
    "contents": {
      "twoColumnSearchResultsRenderer": {
        "primaryContents": {
          "sectionListRenderer": {
            "contents": [
              {
                "itemSectionRenderer": {
                  "contents": [
                    {
                      "videoRenderer": {
                        "videoId": "...",
                        "thumbnail": {
                          "thumbnails": [
                            { "url": "...", "width": 360, "height": 202 },
  			    { "url": "...", "width": 720, "height": 404 }
                          ]
                        },
                        "title": {
                          "runs": [ { "text": "..." } ],
                          "accessibility": { ... }
                        },
                        "lengthText": {
                          "accessibility": { ... },
                          "simpleText": "1:01:02"
                        },
  	            }
  		  },
  		  ...
  		],
  		...
  	      }
  	    },
  	    ...
  	  ],
  	  ...
  	},
  	...
        },
        ...
      },
      ...
    },
    ...
  }
*/

static YoutubeSearchResponse parse_response(String8 body, MemoryArena *arena) {
  if (body.length == 0) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  struct json_tokener *tokener = json_tokener_new_ex(JSON_MAX_DETPH);

  // TODO release this on each error path, refactor error handling
  json_object *json = json_tokener_parse_ex(tokener, body.data, -1);
  enum json_tokener_error json_error = json_tokener_get_error(tokener);

  if (json_error != json_tokener_success) {
    fprintf(stderr, "ERROR: Failed to parse json string: %u\n", json_error);
    json_object_put(json);
    return (YoutubeSearchResponse) { .code = YT_SEARCH_PARSE_ERROR };
  }

  json_object *contents = json_object_object_get(json, "contents");
  if (contents == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *search_results = json_object_object_get(contents, "twoColumnSearchResultsRenderer");
  if (search_results == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *primary_contents = json_object_object_get(search_results, "primaryContents");
  if (primary_contents == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *section_list = json_object_object_get(primary_contents, "sectionListRenderer");
  if (section_list == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *section_contents = json_object_object_get(section_list, "contents");
  if (section_contents == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *first_section = json_object_array_get_idx(section_contents, 0);
  if (first_section == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *item_section = json_object_object_get(first_section, "itemSectionRenderer");
  if (item_section == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  json_object *item_contents = json_object_object_get(item_section, "contents");
  if (item_contents == NULL) {
    return (YoutubeSearchResponse) { .code = YT_SEARCH_NONE };
  }

  usize item_count = json_object_array_length(item_contents);
  usize video_count = 0;
  VideoData videos[item_count];

  for (usize i = 0; i < item_count; i++) {
    json_object *data = json_object_array_get_idx(item_contents, i);
    json_object *video_renderer = json_object_object_get(data, "videoRenderer");

    if (video_renderer != NULL) {
      VideoData video = parse_video_data(video_renderer, arena);
      videos[video_count] = video;
      video_count++;
    }
  }

  VideoData *actual_videos = arena_push(arena, sizeof(VideoData) * video_count);
  for (usize i = 0; i < video_count; i++) {
    actual_videos[i] = videos[i];
  }

  json_object_put(json);
  return (YoutubeSearchResponse) {
    .code = YT_SEARCH_OK,
    .videos = actual_videos,
    .video_count = video_count
  };
}

static VideoData parse_video_data(json_object *video_renderer_json, MemoryArena *arena) {
  String8 id = get_video_id(video_renderer_json, arena);
  String8 title = get_title(video_renderer_json, arena);
  String8 length = get_length(video_renderer_json, arena);
  String8 url = string8_concat(YT_WATCH_URL, id, arena);

  return (VideoData) { .uid = id, .title = title, .length = length, .url = url };
}

static String8 get_video_id(json_object *video_renderer_json, MemoryArena *arena) {
  json_object *id_json = json_object_object_get(video_renderer_json, "videoId");

  if (id_json == NULL) {
    return STRING8("");
  }

  char *id = (char *)json_object_get_string(id_json);
  if (id == NULL) {
    return STRING8("");
  }

  return string8_from_charbuf(id, strlen(id), arena);
}

static String8 get_title(json_object *video_renderer_json, MemoryArena *arena) {
  json_object *title_json = json_object_object_get(video_renderer_json, "title");

  if (title_json == NULL) {
    return STRING8("");
  }

  json_object *runs_json = json_object_object_get(title_json, "runs");
  if (runs_json == NULL) {
    return STRING8("");
  }

  json_object *first_run = json_object_array_get_idx(runs_json, 0);
  if (first_run == NULL) {
    return STRING8("");
  }

  json_object *text_json = json_object_object_get(first_run, "text");
  if (text_json == NULL) {
    return STRING8("");
  }

  char *title = (char *)json_object_get_string(text_json);
  return string8_from_charbuf(title, strlen(title), arena);
}

static String8 get_length(json_object *video_renderer_json, MemoryArena *arena) {
  json_object *length_text = json_object_object_get(video_renderer_json, "lengthText");
  if (length_text == NULL) {
    return STRING8("");
  }

  json_object *simple_text = json_object_object_get(length_text, "simpleText");
  if (simple_text == NULL) {
    return STRING8("");
  }

  char *length = (char *)json_object_get_string(simple_text);
  return string8_from_charbuf(length, strlen(length), arena);
}

#endif
