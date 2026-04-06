
PATH_BUILD = ./build
PATH_BIN =   ./bin
PATH_SRC =   ./src
PATH_EXP =   ./src/exp
PATH_TEST =  ./tst
PATH_UNITY = ./vendor/Unity/src
PATH_UNITY_FIXTURE = ./vendor/Unity/extras/fixture/src
PATH_UNITY_MEMORY  = ./vendor/Unity/extras/memory/src

INCLUDE_DIRS=-I. -I$(PATH_UNITY) -I$(PATH_UNITY_FIXTURE) -I$(PATH_UNITY_MEMORY) -I$(PATH_SRC) -I$(PATH_TEST)
UNITY_SRC_FILES=$(PATH_UNITY)/unity.c $(PATH_UNITY_FIXTURE)/unity_fixture.c $(PATH_UNITY_MEMORY)/unity_memory.c

CC=clang
CFLAGS=-std=c99
CFLAGS_TEST=-g -gdwarf-4
DEBUG_FLAGS=

ifeq ($(DEBUG), true)
  DEBUG_FLAGS += -DDEBUG_MEMORY
endif

ifeq ($(CC), clang)
	CFLAGS_TEST += -fprofile-instr-generate -fcoverage-mapping
	EXEC_TEST = LLVM_PROFILE_FILE="$(PATH_BUILD)/all_tests.profraw" $(PATH_BUILD)/all_tests.out -v
	COVERAGE = llvm-profdata merge -sparse $(PATH_BUILD)/all_tests.profraw -o $(PATH_BUILD)/coverage.profdata 
	COVERAGE += && llvm-cov19 show $(PATH_BUILD)/all_tests.out -ignore-filename-regex='vendor/*' -instr-profile=$(PATH_BUILD)/coverage.profdata -format=html -output-dir=$(PATH_BUILD)/coverage
else ifeq ($(CC), gcc)
	CFLAGS_TEST +=  -fprofile-arcs -ftest-coverage 
	EXEC_TEST = $(PATH_BUILD)/all_tests.out -v
	COVERAGE = lcov --capture --directory $(PATH_BUILD) --output-file $(PATH_BUILD)/coverage.info
	COVERAGE += && lcov --remove $(PATH_BUILD)/coverage.info '/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/' 'vendor' --output-file $(PATH_BUILD)/coverage.filtered.info
	COVERAGE += && genhtml $(PATH_BUILD)/coverage.filtered.info --output-directory $(PATH_BUILD)/coverage
endif
 
SDL2FLAGS=`sdl2-config --cflags --libs`
JSONFLAGS=`pkg-config --cflags --libs json-c`
CURLFLAGS=`pkg-config --cflags --libs libcurl`

glitchz:
	$(CC) $(PATH_EXP)/glitchz.c $(SDL2FLAGS) -o $(PATH_BIN)/glitchz

namedia:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(PATH_SRC)/namedia.c $(SDL2FLAGS) $(JSONFLAGS) $(CURLFLAGS) -o $(PATH_BIN)/namedia

test:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(CFLAGS_TEST) $(INCLUDE_DIRS) $(PATH_TEST)/runners/all_tests.c $(UNITY_SRC_FILES) -o $(PATH_BUILD)/all_tests.out
	- $(EXEC_TEST)
	$(COVERAGE)

test-http:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(PATH_TEST)/test_http.c $(CFLAGS) $(CFLAGS_TEST) $(JSONFLAGS) $(CURLFLAGS) -o $(PATH_BUILD)/test_http.out

test-clean:
	rm $(PATH_BUILD)/*.out
	rm $(PATH_BUILD)/*.*.gcno
	rm $(PATH_BUILD)/*.*.gcda
	rm $(PATH_BUILD)/coverage.*
	rm -r $(PATH_BUILD)/coverage

all: glitchz namedia
