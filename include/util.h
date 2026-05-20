#pragma once

#include <stddef.h>

int read_bytes(int fd, void *buf, size_t size, long offset);
