#ifndef TIMER_H
#define TIMER_H

#include <wos/list.h>

#define MAX_TIMER_REQ		64

struct timer_list {
	unsigned int jiffies;
	void (*fn)(void);
	struct list_head list;
};

#endif
