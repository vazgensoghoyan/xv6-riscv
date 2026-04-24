CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Iinclude -D_XOPEN_SOURCE=700

SRC = src/logger.c src/signals.c
OBJ = $(SRC:.c=.o)

TARGET = log_server

all: $(TARGET)

$(TARGET): main.o $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) main.o $(OBJ)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.o src/*.o $(TARGET)
