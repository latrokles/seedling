
BIN = ./bin

SDL2_FLAGS=`sdl2-config --cflags --libs`
JSON_FLAGS=`pkg-config --cflags --libs json-c`
CURL_FLAGS=`pkg-config --cflags --libs libcurl`

glitchz:
	cc glitchz.c $(SDL2_FLAGS) -o $(BIN)/glitchz

namedia:
	cc namedia.c $(SDL2_FLAGS) $(JSON_FLAGS) $(CURL_FLAGS) -o $(BIN)/namedia

all: glitchz namedia
