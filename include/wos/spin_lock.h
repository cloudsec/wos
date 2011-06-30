#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#define SPIN_LOCK_UNLOCKED              0
#define SPIN_LOCK_LOCKED                1

typedef struct spin_lock {
        unsigned int lock;
}spinlock_t;

void __local_irq_enable(void);
void __local_irq_disable(void);

#define local_irq_enable()              do                                      \
                                                __local_irq_enable();           \
                                        while (0)

#define local_irq_disable()             do                                      \
                                                __local_irq_disable();          \
                                        while (0)


#define local_irq_save(flags)           do                                      \
                                                flags = __local_irq_save();     \
                                        while (0)

#define local_irq_restore(flags)        do                                      \
                                                __local_irq_restore(flags);     \
                                        while (0)

#define spin_lock(lock)                 do                                      \
						local_irq_disable()		\
                                                __spin_lock(lock);              \
                                        while (0)

#define spin_unlock(lock)               do                                      \
                                                __spin_unlock(lock);            \
						local_irq_enable		\
                                        while (0)

#endif
