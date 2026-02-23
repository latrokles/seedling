
BIN = ./bin
EXP = ./exp
SRC = ./src
TEST = ./tst

SDL2_FLAGS=`sdl2-config --cflags --libs`
JSON_FLAGS=`pkg-config --cflags --libs json-c`
CURL_FLAGS=`pkg-config --cflags --libs libcurl`

glitchz:
	cc $(EXP)/glitchz.c $(SDL2_FLAGS) -o $(BIN)/glitchz

namedia:
	cc $(SRC)/namedia.c $(SDL2_FLAGS) $(JSON_FLAGS) $(CURL_FLAGS) -o $(BIN)/namedia


test_http:
	cc $(TEST)/test_http.c $(SRC)/http.c $(JSON_FLAGS) $(CURL_FLAGS) -o $(TEST)/test_http 
	./$(TEST)/test_http
	rm $(TEST)/test_http

test: test_http

all: glitchz namedia
