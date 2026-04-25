#ifndef SERVER_H
#define SERVER_H

void ensure_fifo(const char *path);
void handle_async_events(void);
void daemonize_if_needed(void);

#endif
