CC = gcc

CFLAGS = -Wall -Wextra -g -Iinclude

OBJ = \
    src/ext2.o \
    src/util.o

all: sb_info inode_info inode_cat

sb_info: $(OBJ) src/sb_info.o
	$(CC) $(CFLAGS) -o $@ $^

inode_info: $(OBJ) src/inode_info.o
	$(CC) $(CFLAGS) -o $@ $^

inode_cat: $(OBJ) src/inode_cat.o
	$(CC) $(CFLAGS) -o $@ $^

test:
	bash tests/test.sh

clean:
	rm -f src/*.o sb_info inode_info inode_cat
