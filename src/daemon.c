#include "daemon.h"
#include "logger.h"
#include <unistd.h>
#include <stdlib.h>

static int is_daemon = 0;

void daemonize_if_needed(void) {
    if (is_daemon)
        return;

    is_daemon = 1;

    if (fork() > 0) exit(0);
    setsid();

    if (fork() > 0) exit(0);

    freopen("/tmp/log_server_daemon.log", "a", stdout);
    freopen("/tmp/log_server_daemon.log", "a", stderr);

    log_msg("daemonized via SIGHUP\n");
}
