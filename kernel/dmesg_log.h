#ifndef _DMESG_LOG_H_
#define _DMESG_LOG_H_

#define LOG_SYSCALL_MASK	(1 << 0)
#define LOG_INTR_MASK		(1 << 1)
#define LOG_PROC_MASK		(1 << 2)
#define LOG_EXEC_MASK		(1 << 3)

#define LOG_SYSCALL_MSG(...) \
	do { \
		if (log_enabled(LOG_SYSCALL_MASK)) \
			pr_msg(__VA_ARGS__); \
	} while (0)

#define LOG_INTR_MSG(...) \
	do { \
		if (log_enabled(LOG_INTR_MASK)) \
			pr_msg(__VA_ARGS__); \
	} while (0)

#define LOG_PROC_MSG(...) \
	do { \
		if (log_enabled(LOG_PROC_MASK)) \
			pr_msg(__VA_ARGS__); \
	} while (0)

#define LOG_EXEC_MSG(...) \
	do { \
		if (log_enabled(LOG_EXEC_MASK)) \
			pr_msg(__VA_ARGS__); \
	} while (0)

void loginit(void);
int log_enabled(int type);

#endif
