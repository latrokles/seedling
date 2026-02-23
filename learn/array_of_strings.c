#include <stdio.h>

void print_strings(char *s[], size_t count) {
  printf("count=%zu\n", count);
  for (size_t i = 0; i < count; i++) {
    printf("index=%zu, value=%s\n", i, s[i]);
  }
}

int main(void) {
  char *strings[] = {
    "foo",
    "bar",
    "baz"
  };

  // this seems to only be possible where the array is declared?
  size_t count = sizeof(strings) / sizeof(strings[0]);

  print_strings(strings, count);
}
