#define	syscall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) { \
		errno = -(res); \
		res = -1; \
	} \
	return (type) (res); \
} while (0)

#define syscall0(type, name) \
type name(void) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
	:"=a"(__res) \
    	:"0"(__NR_##name)); \
__syscall_return(type,__res); \
}

#define syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
long __res; \
__asm__ volatile ("push %%ebx ; movl %2,%%ebx ; int $0x80 ; pop %%ebx" \
	:"=a"(__res) \
	:"0"(__NR_##name),"ri" ((long)(arg1)) : "memory"); \
syscall_return(type,__res); \
}
