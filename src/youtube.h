#include <curl/curl.h>

typedef enum YouTubeSearchReturnCode {
  YT_RESULT_OK,
  YT_RESULT_API_ERROR,
  YT_RESULT_PARSE_ERROR
} YouTubeSearchReturnCode;

typedef struct MediaInfo {
  char *uid;
  char *title;
  char *length;
} MediaInfo ;

typedef YouTubeSearchResults {
  YouTubeSearchReturnCode code;
  MediaInfo results[];
} YouTubeSearchResults;


YouTubeSearchResults* perform_youtube_search(CURL *curl, char query[]);
