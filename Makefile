
PATH_BUILD = ./build
PATH_BIN =   ./bin
PATH_SRC =   ./src
PATH_EXP =   ./exp
PATH_TEST =  ./tst
PATH_UNITY = ./vendor/Unity/src
PATH_UNITY_FIXTURE = ./vendor/Unity/extras/fixture/src
PATH_UNITY_MEMORY  = ./vendor/Unity/extras/memory/src

INCLUDE_DIRS=-I. -I$(PATH_UNITY) -I$(PATH_UNITY_FIXTURE) -I$(PATH_UNITY_MEMORY) -I$(PATH_SRC) -I$(PATH_TEST)
UNITY_SRC_FILES=$(PATH_UNITY)/unity.c $(PATH_UNITY_FIXTURE)/unity_fixture.c $(PATH_UNITY_MEMORY)/unity_memory.c

CFLAGS=-std=c99
TEST_CFLAGS=-g -gdwarf-4 -fprofile-arcs -ftest-coverage 
SDL2FLAGS=`sdl2-config --cflags --libs`
JSONFLAGS=`pkg-config --cflags --libs json-c`
CURLFLAGS=`pkg-config --cflags --libs libcurl`

glitchz:
	cc $(PATH_EXP)/glitchz.c $(SDL2FLAGS) -o $(PATH_BIN)/glitchz

namedia:
	cc $(PATH_SRC)/namedia.c $(SDL2FLAGS) $(JSONFLAGS) $(CURLFLAGS) -o $(PATH_BIN)/namedia

test:
	cc $(CFLAGS) $(TEST_CFLAGS) $(INCLUDE_DIRS) $(PATH_TEST)/runners/all_tests.c $(UNITY_SRC_FILES) -o $(PATH_BUILD)/all_tests.out
	- $(PATH_BUILD)/all_tests.out -v
	lcov --capture --directory $(PATH_BUILD) --output-file $(PATH_BUILD)/coverage.info
	lcov --remove $(PATH_BUILD)/coverage.info '/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/' 'vendor' \
		--output-file $(PATH_BUILD)/coverage.filtered.info
	genhtml $(PATH_BUILD)/coverage.filtered.info --output-directory $(PATH_BUILD)/coverage

test-http:
	cc $(PATH_TEST)/test_http.c $(CFLAGS) $(TEST_CFLAGS) $(JSONFLAGS) $(CURLFLAGS) -o $(PATH_BUILD)/test_http.out

test-clean:
	rm $(PATH_BUILD)/*.out
	rm $(PATH_BUILD)/*.*.gcno
	rm $(PATH_BUILD)/*.*.gcda
	rm $(PATH_BUILD)/coverage.*
	rm -r $(PATH_BUILD)/coverage

all: glitchz namedia
