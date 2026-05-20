#pragma once

#include <stddef.h>
#include <sys/types.h>

int read_bytes(int fd, void *buf, size_t size, off_t offset);
