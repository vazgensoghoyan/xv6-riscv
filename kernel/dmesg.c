#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

// STRUCT for dmesg

struct {
    struct spinlock lock;
    char buf[DMESG_SIZE];
    uint head;
    uint tail;
} dmesg;

// DMESG init and putc

void dmesginit(void) {
    initlock(&dmesg.lock, "dmesg");
    dmesg.head = 0;
    dmesg.tail = 0;
}

static void dmesg_putc(char c) {
    dmesg.buf[dmesg.head] = c;
    dmesg.head = (dmesg.head + 1) % DMESG_SIZE;

    if (dmesg.head == dmesg.tail) {
        dmesg.tail = (dmesg.tail + 1) % DMESG_SIZE;
    }
}

// DMESG PRINT realization: just almost copypast of printf

extern uint ticks;
extern struct spinlock tickslock;

// helpers

static char digits[] = "0123456789abcdef";

static void dmesg_printint(long long xx, int base, int sign) {
	char buf[20];
	int i;
	unsigned long long x;

	if(sign && (sign = (xx < 0)))
		x = -xx;
	else
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		dmesg_putc(buf[i]);
}

static void dmesg_printptr(uint64 x) {
	int i;
	dmesg_putc('0');
	dmesg_putc('x');
	for(i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
		dmesg_putc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// the function

void pr_msg(const char *fmt, ...) {
	va_list ap;
	int i, cx, c0, c1, c2;
	char *s;

	acquire(&dmesg.lock);

	acquire(&tickslock);
	uint t = ticks;
	release(&tickslock);

	dmesg_putc('[');
	dmesg_printint(t, 10, 0);
	dmesg_putc(']');
	dmesg_putc(' ');

	va_start(ap, fmt);
	for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
		if(cx != '%'){
			dmesg_putc(cx);
			continue;
		}
		i++;
		c0 = fmt[i+0] & 0xff;
		c1 = c2 = 0;
		if(c0) c1 = fmt[i+1] & 0xff;
		if(c1) c2 = fmt[i+2] & 0xff;
		if(c0 == 'd'){
			dmesg_printint(va_arg(ap, int), 10, 1);
		} else if(c0 == 'l' && c1 == 'd'){
			dmesg_printint(va_arg(ap, uint64), 10, 1);
			i += 1;
		} else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
			dmesg_printint(va_arg(ap, uint64), 10, 1);
			i += 2;
		} else if(c0 == 'u'){
			dmesg_printint(va_arg(ap, uint32), 10, 0);
		} else if(c0 == 'l' && c1 == 'u'){
			dmesg_printint(va_arg(ap, uint64), 10, 0);
			i += 1;
		} else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
			dmesg_printint(va_arg(ap, uint64), 10, 0);
			i += 2;
		} else if(c0 == 'x'){
			dmesg_printint(va_arg(ap, uint32), 16, 0);
		} else if(c0 == 'l' && c1 == 'x'){
			dmesg_printint(va_arg(ap, uint64), 16, 0);
			i += 1;
		} else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
			dmesg_printint(va_arg(ap, uint64), 16, 0);
			i += 2;
		} else if(c0 == 'p'){
			dmesg_printptr(va_arg(ap, uint64));
		} else if(c0 == 'c'){
			dmesg_putc(va_arg(ap, uint));
		} else if(c0 == 's'){
			if((s = va_arg(ap, char*)) == 0)
				s = "(null)";
			for(; *s; s++)
				dmesg_putc(*s);
		} else if(c0 == '%'){
			dmesg_putc('%');
		} else if(c0 == 0){
			break;
		} else {
			dmesg_putc('%');
			dmesg_putc(c0);
		}
	}
	va_end(ap);

	dmesg_putc('\n');

	release(&dmesg.lock);
}
