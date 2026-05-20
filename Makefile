CC = gcc

CFLAGS = -Wall -Wextra -g -Iinclude

OBJ = \
    src/ext2.o \
    src/util.o

all: sb_info inode_info

sb_info: $(OBJ) src/sb_info.o
	$(CC) $(CFLAGS) -o $@ $^

inode_info: $(OBJ) src/inode_info.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o sb_info inode_info
