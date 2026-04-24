#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void log_init_foreground(void);
void log_init_file(const char *filename);
void log_msg(const char *msg);
void log_close(void);

#endif
