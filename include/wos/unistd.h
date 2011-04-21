#ifndef UNISTD_H
#define UNISTD_H

#define __NR_write		0
#define __NR_fork		1
#define __NR_pause		2
#define __NR_creat_task		3

#define _syscall0(type, name)				\
type name(void)						\
{							\
	long ret;					\
	asm("int $0x85":"=a"(ret):"0"(__NR_##name));	\
	if (ret >= 0)					\
		return (type)ret;			\
	return -1;					\
}

#define _syscall1(type, name, type1, arg1)		\
type name(type1 arg1)					\
{							\
	long ret;					\
	asm("int $0x85":"=a"(ret):"0"(__NR_##name),	\
		"b"((long)(arg1)));			\
	if (ret >= 0)					\
		return (type)ret;			\
	return -1;					\
}

#endif
