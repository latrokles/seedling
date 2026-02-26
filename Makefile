
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


test_str8:
	cc $(TEST)/test_str8.c -o $(TEST)/test_str8
	$(TEST)/test_str8
	rm $(TEST)/test_str8

test_http:
	cc $(TEST)/test_http.c $(SRC)/http.c $(JSON_FLAGS) $(CURL_FLAGS) -o $(TEST)/test_http 
	./$(TEST)/test_http
	rm $(TEST)/test_http

test: test_http

all: glitchz namedia
