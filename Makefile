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
TOOLS := $(patsubst $(CMD_DIR)/%/main.c, $(BUILD_DIR)/%, $(CMDS))

.PHONY: all clean make-build-dir

all: make-build-dir $(TOOLS)

$(BUILD_DIR)/%: $(CMD_DIR)/%/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(DEPS) $< -o $@

clean:
	rm -r $(BUILD_DIR)/*

make-build-dir:
	mkdir -p $(BUILD_DIR)
