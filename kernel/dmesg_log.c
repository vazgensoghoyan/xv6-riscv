#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#include "dmesg_log.h"

struct {
	struct spinlock lock;
	int mask;
	uint until;
} logctl;

extern uint ticks;
extern struct spinlock tickslock;

void loginit(void) {
	initlock(&logctl.lock, "logctl");

#ifdef LOG_SYSCALL
	logctl.mask |= LOG_SYSCALL_MASK;
#endif
#ifdef LOG_INTR
	logctl.mask |= LOG_INTR_MASK;
#endif
#ifdef LOG_PROC
	logctl.mask |= LOG_PROC_MASK;
#endif
#ifdef LOG_EXEC
	logctl.mask |= LOG_EXEC_MASK;
#endif

	logctl.until = 0;
}

int log_enabled(int type) {
	int enabled;

	acquire(&logctl.lock);

	if (logctl.until != 0) {
		acquire(&tickslock);

		if (ticks >= logctl.until) {
			logctl.mask = 0;
			logctl.until = 0;
		}

		release(&tickslock);
	}

	enabled = (logctl.mask & type);

	release(&logctl.lock);

	return enabled;
}

uint64 sys_logctl(void) {
	int mask;
	int timeout;

	argint(0, &mask);
	argint(1, &timeout);

	acquire(&logctl.lock);

	logctl.mask = mask;

	if (timeout > 0) {
		acquire(&tickslock);
		logctl.until = ticks + timeout;
		release(&tickslock);
	} else {
		logctl.until = 0;
	}

	release(&logctl.lock);

	return 0;
}
