CC = gcc

CFLAGS = -Wall -Wextra -g -std=gnu11 -Iinclude
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build

OBJ = \
    $(BUILD_DIR)/ext2.o \
    $(BUILD_DIR)/util.o

all: sb_info inode_info inode_cat

sb_info: $(OBJ) $(BUILD_DIR)/sb_info.o
	$(CC) $(LDFLAGS) -o $@ $^

inode_info: $(OBJ) $(BUILD_DIR)/inode_info.o
	$(CC) $(LDFLAGS) -o $@ $^

inode_cat: $(OBJ) $(BUILD_DIR)/inode_cat.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test:
	bash tests/test.sh

valgrind:
	valgrind --leak-check=full ./inode_cat ext2.img 2

clean:
	rm -rf $(BUILD_DIR) sb_info inode_info inode_cat
