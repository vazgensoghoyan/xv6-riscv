CC = gcc

CFLAGS = -Wall -Wextra -g -Iinclude

OBJ = \
    src/ext2.o \
    src/util.o

all: sbinfo

sbinfo: $(OBJ) src/sbinfo.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o sbinfo
