CC = gcc
CFLAGS = -std=c17 \
    -Wall -Wextra -Wpedantic -Wshadow \
    -Wconversion -Wformat=2 \
    -fanalyzer \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -g -O1 \
    -pthread \
    -D_POSIX_C_SOURCE=200809L

LDFLAGS = -fsanitize=address,undefined -pthread

TARGET_EXEC = tinycache

BUILD_DIR = build
SRC_DIR = src

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)