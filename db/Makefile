CC = gcc

TARGET_EXEC = tinycache

BUILD_DIR = build
SRC_DIR = src

SRCS = $(shell find $(SRC_DIR) -name '*.c')

# common flags
CFLAGS = -std=c17 \
    -Wall -Wextra -Wpedantic \
    -Wshadow -Werror -Wconversion -Wformat=2 \
    -pthread \
    -D_POSIX_C_SOURCE=200809L

LDFLAGS = -pthread

# debug flags
DEBUG_CFLAGS = -O1 -g \
    -fanalyzer \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -DTS_DB_DBG

DEBUG_LDFLAGS = -fsanitize=address,undefined

# race test flags
RACE_TEST_CFLAGS = -O1 -g \
    -fsanitize=thread \
    -DTS_DB_DBG

# release flags
RELEASE_CFLAGS = -O2 -DNDEBUG
RACE_TEST_LDFLAGS = -fsanitize=thread


.PHONY: all release debug race clean
all: release

release: BUILD_TYPE = release
release: EXTRA_CFLAGS = $(RELEASE_CFLAGS)
release: EXTRA_LDFLAGS = $(RELEASE_LDFLAGS)
release: build

debug: BUILD_TYPE = debug
debug: EXTRA_CFLAGS = $(DEBUG_CFLAGS)
debug: EXTRA_LDFLAGS = $(DEBUG_LDFLAGS)
debug: build

race: BUILD_TYPE = race
race: EXTRA_CFLAGS = $(RACE_CFLAGS)
race: EXTRA_LDFLAGS = $(RACE_LDFLAGS)
race: build

build: $(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET_EXEC)

$(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET_EXEC): $(SRCS)
	@mkdir -p $(BUILD_DIR)/$(BUILD_TYPE)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(SRCS) \
	    -o $@ $(LDFLAGS) $(EXTRA_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)