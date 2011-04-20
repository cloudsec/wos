#ifndef SCHED_H
#define SCHED_H

#define switch_task(next)							\
	struct {long a, b;} tmp;						\
	if (current == next)							\
		return ;							\
        current = next;								\
        asm("movw %%dx, %1\n\t"							\
                "ljmp %0\n"							\
                ::"m"(*&tmp.a), "m"(*&tmp.b), "d"(next->tss_sel));


#endif
