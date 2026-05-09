#include "kernel/types.h"
#include "user/user.h"

#define LOG_SYSCALL_MASK	(1 << 0)
#define LOG_INTR_MASK		(1 << 1)
#define LOG_PROC_MASK		(1 << 2)
#define LOG_EXEC_MASK		(1 << 3)

static int parse_flag(char *s) {
	if(strcmp(s, "syscall") == 0) return LOG_SYSCALL_MASK;
	if(strcmp(s, "intr") == 0) return LOG_INTR_MASK;
	if(strcmp(s, "proc") == 0) return LOG_PROC_MASK;
	if(strcmp(s, "exec") == 0) return LOG_EXEC_MASK;
	if(strcmp(s, "all") == 0) return LOG_SYSCALL_MASK|LOG_INTR_MASK|LOG_PROC_MASK|LOG_EXEC_MASK;
	return -1;
}

int main(int argc, char *argv[]) {
	if(argc < 2){
		printf("usage:\n");
		printf("  logctl on <flags> [ticks]\n");
		printf("  logctl off <flags>\n");
		printf("  logctl status\n");
		exit(1);
	}

	if(strcmp(argv[1], "status") == 0){
		printf("logging control via kernel\n");
		exit(0);
	}

	int mask = 0;
	int timeout = 0;

	if(strcmp(argv[1], "on") == 0){
		if(argc < 3){
			printf("no flags\n");
			exit(1);
		}

		char *p = argv[2];
		char *token;

		while((token = strchr(p, ',')) != 0){
			*token = 0;
			int f = parse_flag(p);
			if(f < 0) exit(1);
			mask |= f;
			p = token + 1;
		}

		int f = parse_flag(p);
		if(f < 0) exit(1);
		mask |= f;

		if(argc >= 4)
			timeout = atoi(argv[3]);

		logctl(mask, timeout);
		exit(0);
	}

	if(strcmp(argv[1], "off") == 0){
		if(argc < 3){
			logctl(0, 0);
			exit(0);
		}

		char *p = argv[2];
		char *token;

		while((token = strchr(p, ',')) != 0){
			*token = 0;
			int f = parse_flag(p);
			if(f < 0) exit(1);
			mask |= f;
			p = token + 1;
		}

		int f = parse_flag(p);
		if(f < 0) exit(1);
		mask |= f;

		logctl(~mask, 0);
		exit(0);
	}

	printf("unknown command\n");
	exit(1);
}
