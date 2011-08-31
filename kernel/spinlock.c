/*
 * spinlock.c - SMP spin lock support.
 *
 * (c) wzt 2011		http://www.cloud-sec.org
 *
 */
 
#include <wos/spin_lock.h>

void __local_irq_enable(void)
{
	__asm__ __volatile__("sti"::);
}

void __local_irq_disable(void)
{
	__asm__ __volatile__("cli"::);
}

unsigned int  __local_irq_save(void)
{
	unsigned int flags;

	__asm__ __volatile__("pushfl\n\t"
				"popl %0":"=m"(flags));

	return flags;
}

void __local_irq_restore(unsigned int flags)
{
	__asm__ __volatile__("pushl %0\n\t"
				"popfl"::"m"(flags));
}

void init_spin_lock(spinlock_t *spin_lock)
{
        spin_lock->lock = SPIN_LOCK_UNLOCKED;
}

void __spin_lock(spinlock_t *spin_lock)
{
        asm("1:\n\t"
                "lock\n\t"
                "btsl $0, %0\n\t"
                "jnc 3f\n\t"
                "2:\n\t"
                "rep\n\t"
                "nop\n\t"
                "testl $1, %0\n\t"
                "jz 2b\n\t"
                "jmp 1b\n\t"
                "3:"
                ::"m"(spin_lock->lock):"memory");
}

void __spin_unlock(spinlock_t *spin_lock)
{
        asm("lock\n\t"
                "btcl $0, %0"::"m"(spin_lock->lock):"memory");
}
