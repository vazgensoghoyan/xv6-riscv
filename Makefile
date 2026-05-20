CC = gcc

CFLAGS = -Wall -Wextra -g -std=gnu11 -Iinclude

SRC_DIR = src
BUILD_DIR = build

UTILS = sb_info inode_info inode_cat inode_dir

OBJ = \
    $(BUILD_DIR)/ext2.o \
    $(BUILD_DIR)/util.o

all: $(UTILS)

$(UTILS): %: $(OBJ) $(BUILD_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test:
	bash tests/test.sh

clean:
	rm -rf $(BUILD_DIR) $(UTILS)
