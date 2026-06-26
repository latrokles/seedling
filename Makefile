## shared variables

CC = clang
CFLAGS = -std=c99
DEBUG_FLAGS =

# build artifacts
BIN_PREFIX ?= $(HOME)
BIN_DIR = $(BIN_PREFIX)/bin
BUILD_DIR = ./build

# sources
SRC_DIR = ./src
CMD_DIR = $(SRC_DIR)/cmd
EXP_DIR = $(SRC_DIR)/exp
LIB_DIR = $(SRC_DIR)/lib

INCLUDES = -I$(LIB_DIR) -I$(CMD_DIR)

# dependency libraries
FT   = `pkg-config --cflags --libs freetype2`
SDL2 = `sdl2-config --cflags --libs`
JSON = `pkg-config --cflags --libs json-c`
CURL = `pkg-config --cflags --libs libcurl`
DEPS = $(FT) $(SDL2) $(JSON) $(CURL)

CMDS := $(wildcard $(CMD_DIR)/*/main.c)
TOOLS := $(patsubst $(CMD_DIR)/%/main.c, $(BUILD_DIR)/bin/%, $(CMDS))

EXPS := $(wildcard $(EXP_DIR)/*.c)
EXP_BINS = $(patsubst $(EXP_DIR)/%.c, $(BUILD_DIR)/exp/%, $(EXPS))

.PHONY: all clean make-build-dir

all: make-build-dir $(TOOLS)

$(BUILD_DIR)/bin/%: $(CMD_DIR)/%/main.c
	@mkdir -p $(BUILD_DIR)/bin
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(DEPS) $< -o $@

$(BUILD_DIR)/exp/%: $(EXP_DIR)/%.c
	@mkdir -p $(BUILD_DIR)/exp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(DEPS) $< -o $@

clean:
	rm -r $(BUILD_DIR)/*

make-build-dir:
	mkdir -p $(BUILD_DIR)/bin
	mkdir -p $(BUILD_DIR)/exp


################
### TEST TARGETS
################

# testing
TEST_CASES        = test_all # test_http
TEST_DIR          = ./tst
TEST_INCLUDES     = -I$(TEST_DIR)
TEST_BUILD_DIR    = $(BUILD_DIR)/test

UNITY_DIR         = ./vendor/Unity/src
UNITY_FIXTURE_DIR = ./vendor/Unity/extras/fixture/src
UNITY_MEMORY_DIR  = ./vendor/Unity/extras/memory/src

UNITY_SRC      = $(UNITY_DIR)/unity.c $(UNITY_FIXTURE_DIR)/unity_fixture.c $(UNITY_MEMORY_DIR)/unity_memory.c
UNITY_INCLUDES = -I$(UNITY_DIR) -I$(UNITY_FIXTURE_DIR) -I$(UNITY_MEMORY_DIR)

TEST_FLAGS = -g -gdwarf-4

ifeq ($(CC), clang)
# when compiling with clang use llvm-cov to generate code coverage report
	TEST_FLAGS += -fprofile-instr-generate -fcoverage-mapping
	EXEC_PREFIX = LLVM_PROFILE_FILE="$(TEST_BUILD_DIR)/test_suite.profraw"
	TEST_CODE_COV = llvm-profdata merge -sparse $(TEST_BUILD_DIR)/test_suite.profraw -o $(TEST_BUILD_DIR)/test_suite.profdata
	TEST_CODE_COV_HTML = llvm-cov19 show $(TEST_BUILD_DIR)/test_suite.out                     \
                                             -ignore-filename-regex='vendor/*'               \
                                             -ignore-filename-regex='usr/local/include/*'    \
                                             -instr-profile=$(TEST_BUILD_DIR)/test_suite.profdata \
                                             -output-dir=$(TEST_BUILD_DIR)/coverage               \
                                             -format=html
else ifeq ($(CC), gcc)
# when compiling with gcc use lcov and genhtml to generate code coverage report
	TEST_FLAGS += -fprofile-arcs -ftest-coverage
	EXEC_PREFIX=
	TEST_CODE_COV = lcov --capture --directory $(TEST_BUILD_DIR) --output-file $(TEST_BUILD_DIR)/test_suite.info
	TEST_CODE_COV_HTML = lcov --remove $(TEST_BUILD_DIR)/test_suite.info                                       \
                                           '/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/' \
                                           'vendor'                                                           \
                                           --output-file $(TEST_BUILD_DIR)/test_suite-filtered.info &&             \
                                           genhtml $(TEST_BUILD_DIR)/test_suite-filtered.info --output-directory $(TEST_BUILD_DIR)/coverage
endif

test: make-build-dir
	@mkdir -p $(TEST_BUILD_DIR)

# compile test suite
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_FLAGS) $(INCLUDES)       \
                                                     $(TEST_INCLUDES)  \
                                                     $(UNITY_INCLUDES) \
                                                     $(UNITY_SRC)      \
                                                     $(DEPS)           \
	                                             $(TEST_DIR)/runners/test_suite.c \
                                                     -o $(TEST_BUILD_DIR)/test_suite.out

# execute test suite
	$(EXEC_PREFIX) $(TEST_BUILD_DIR)/test_suite.out -v

# profile/compute code coverage
	$(TEST_CODE_COV)

# generate profiling/code coverage html report
	$(TEST_CODE_COV_HTML)
