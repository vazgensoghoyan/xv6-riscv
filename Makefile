CC=gcc
CFLAGS=-Wall -Wextra -O2 -Iinclude

SRC=src/*.c main.c
OUT=log_server

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
