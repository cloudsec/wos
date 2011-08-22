#ifndef TASK_H
#define TASK_H

#include <wos/list.h>
#include <wos/gdt.h>

struct tss_struct {
	unsigned int prev_task_link;
	unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	unsigned int fs;
	unsigned int gs;
	unsigned int ldt_sel;
	unsigned int io_map;
};

struct task_struct {
	struct tss_struct tss;
	unsigned int tss_sel;
	unsigned int ldt_sel;
	struct gdt_desc ldt[3];
	int pid;
	int state;
	int counter;
	int priority;
	struct list_head list;
};

struct regs {
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        unsigned int esi;
        unsigned int edi;
        unsigned int ebp;
        unsigned int eax;
        unsigned int ds;
        unsigned int es;
        unsigned int fs;
        unsigned int error_code;
        unsigned int orig_eip;
        unsigned int orig_cs;
        unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
};

#define TSS_SEL_BASE		4

#define TSS_IDX(nr)		(TSS_SEL_BASE + 2*nr)
#define LDT_IDX(nr)		(TSS_SEL_BASE + 2*nr + 1)

#define TSS_SEL(nr)		(TSS_IDX(nr) << 3)
#define LDT_SEL(nr)		(LDT_IDX(nr) << 3)

#define TASK_STOP		-1
#define TASK_RUNABLE		0
#define TASK_RUNNING		1	

#define DEFAULT_PRIORITY	200
//#define DEFAULT_COUNTER		200
#define DEFAULT_COUNTER		15

#define MAX_PID			1024

#define MOVE_TO_RING3()		asm("movl %%esp, %%eax\n\t"	\
					"pushl $0x17\n\t"	\
					"pushl %%eax\n\t"	\
					"pushfl\n\t"		\
					"pushl $0x0f\n\t"	\
					"pushl $1f\n\t"		\
					"iret\n\t"		\
					"1:\n\t"		\
					"movw $0x17, %%ax\n\t"	\
					"movw %%ax, %%ds\n\t"	\
					"movw %%ax, %%es\n\t"	\
					"movw %%ax, %%fs\n\t"	\
					"movw %%ax, %%gs\n"	\
					::);

struct list_head task_list_head;
struct task_struct *current;

int get_pid(void);
int creat_kthread(unsigned int eip);

#endif
