
## vars

# compiler
CC          = clang
CFLAGS      = -std=c99
DEBUG_FLAGS =

#ifdef DEBUG
#  for flag in $(DEBUG); do \
#    DEBUG_FLAGS += -DDEBUG_$(flag) ; \
#  done
#endif

# build artifact
BIN_PREFIX ?= $(HOME)
BIN_DIR   = $(BIN_PREFIX)/bin
BUILD_DIR = ./build

# sources

SRC_DIR      = ./src
SRC_CMD_DIR  = $(SRC_DIR)/cmd
SRC_STD_DIR  = $(SRC_DIR)/std
SRC_SYS_DIR  = $(SRC_DIR)/sys
SRC_INCLUDES = -I$(SRC_CMD_DIR) -I$(SRC_STD_DIR) -I$(SRC_SYS_DIR)

# libs
SDL2 = `sdl2-config --cflags --libs`
JSON = `pkg-config --cflags --libs json-c`
CURL = `pkg-config --cflags --libs libcurl`

# testing
TEST_CASES        = test_all # test_http
TEST_DIR          = ./tst
TEST_INCLUDES     = -I$(TEST_DIR)

UNITY_DIR         = ./vendor/Unity/src
UNITY_FIXTURE_DIR = ./vendor/Unity/extras/fixture/src
UNITY_MEMORY_DIR  = ./vendor/Unity/extras/memory/src

UNITY_SRC      = $(UNITY_DIR)/unity.c $(UNITY_FIXTURE_DIR)/unity_fixture.c $(UNITY_MEMORY_DIR)/unity_memory.c
UNITY_INCLUDES = -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_MEMORY_DIR)

TEST_FLAGS = -g -gdwarf-4

ifeq ($(CC), clang)
# when compiling with clang use llvm-cov to generate code coverage report
	TEST_FLAGS += -fprofile-instr-generate -fcoverage-mapping
	EXEC_PREFIX = LLVM_PROFILE_FILE="$(BUILD_DIR)/test_suite.profraw"
	TEST_CODE_COV = llvm-profdata merge -sparse $(BUILD_DIR)/test_suite.profraw -o $(BUILD_DIR)/test_suite.profdata
	TEST_CODE_COV_HTML = llvm-cov19 show $(BUILD_DIR)/test_suite.out                     \
                                             -ignore-filename-regex='vendor/*'               \
                                             -ignore-filename-regex='usr/local/include/*'    \
                                             -instr-profile=$(BUILD_DIR)/test_suite.profdata \
                                             -output-dir=$(BUILD_DIR)/coverage               \
                                             -format=html
else ifeq ($(CC), gcc)
# when compiling with gcc use lcov and genhtml to generate code coverage report
	TEST_FLAGS += -fprofile-arcs -ftest-coverage
	EXEC_PREFIX=
	TEST_CODE_COV = lcov --capture --directory $(BUILD_DIR) --output-file $(BUILD_DIR)/test_suite.info
	TEST_CODE_COV_HTML = lcov --remove $(BUILD_DIR)/test_suite.info                                       \
                                           '/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/' \
                                           'vendor'                                                           \
                                           --output-file $(BUILD_DIR)/test_suite-filtered.info &&             \
                                           genhtml $(BUILD_DIR)/test_suite-filtered.info --output-directory $(BUILD_DIR)/coverage
endif

# binaries
BINARIES = namedia

## targets
.PHONY: all clean install make-build-dir test


all: make-build-dir
	@for bin in $(BINARIES); do \
		$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC_INCLUDES) $(SRC_CMD_DIR)/$$bin.c $(SDL2) $(JSON) $(CURL) -o $(BUILD_DIR)/$$bin; \
	done

clean:
	rm -r $(BUILD_DIR)/*

install: all
	@for bin in $(BINARIES); do \
		cp $(BUILD_DIR)/$$bin $(BIN_DIR)/ ; \
	done

make-build-dir:
	mkdir -p $(BUILD_DIR)

test: make-build-dir
# compile test suite
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_FLAGS) $(SRC_INCLUDES)   \
                                                     $(TEST_INCLUDES)  \
                                                     $(UNITY_INCLUDES) \
                                                     $(UNITY_SRC)      \
                                                     $(SDL2)           \
                                                     $(JSON)           \
                                                     $(CURL)           \
	                                             $(TEST_DIR)/runners/test_suite.c \
                                                     -o $(BUILD_DIR)/test_suite.out

# execute test suite
	$(EXEC_PREFIX) $(BUILD_DIR)/test_suite.out -v

# profile/compute code coverage
	$(TEST_CODE_COV)

# generate profiling/code coverage html report
	$(TEST_CODE_COV_HTML)
