#ifndef FIFO_H
#define FIFO_H

int fifo_open_blocking(const char *path);
int fifo_read_loop(int fd, char *buf, int buf_size);

void fifo_close(int fd);

#endif
