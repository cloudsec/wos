#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/string.h>

extern struct task_struct init_task;
extern void default_int(void);
extern void run_init_task(void);
extern void run_init_task1(void);
extern void run_init_task2(void);

struct task_struct task1;
unsigned int task1_stack0[1024];
unsigned int task1_stack3[1024];

struct task_struct task2;
unsigned int task2_stack0[1024];
unsigned int task2_stack3[1024];

extern unsigned int init_task_stack_ring3[1024];

static inline _syscall0(int, fork)
static inline _syscall0(int, pause)
static inline _syscall1(int, write, char*, msg)
static inline _syscall1(int, creat_task, unsigned int, eip)

asm(".align 4\n");
struct gdt_desc new_gdt[8192] = {0,};
unsigned int gdt_desc_idx = KERNEL_CODE_IDX;
	
void __set_gdt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit, 
		int type_h, int type, int idx)
{
	struct gdt_desc *gdt = addr + idx;
	unsigned int a, b;

	gdt->a = (base << 16) | (0x0000ffff & limit);

	a = ((base >> 16) & 0x00ff) | (type << 8);
	b = (base & 0xff000000) | (type_h << 20) | (limit & 0x000f0000);
	gdt->b = a | b;
}

void set_gdt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit, 
                int type, int idx)
{
	__set_gdt_desc(addr, base, limit, DESC_DEFAULT_TYPE, type, idx);
}

void set_ldt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit, 
		int type, int idx)
{
	__set_gdt_desc(addr, base, limit, DESC_SYSTEM_TYPE, type, idx);
}

void set_tss_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit, 
		int type, int idx)
{
	__set_gdt_desc(addr, base, limit, DESC_SYSTEM_TYPE, type, idx);
}


void setup_gdt(void)
{
	set_gdt_desc(new_gdt, CODE_BASE, CODE_LIMIT, KERNEL_CODE_TYPE, KERNEL_CODE_IDX);
	set_gdt_desc(new_gdt, DATA_BASE, DATA_LIMIT, KERNEL_DATA_TYPE, KERNEL_DATA_IDX);
	set_gdt_desc(new_gdt, VIDEO_BASE, VIDEO_LIMIT, VIDEO_TYPE, VIDEO_IDX);
}

void print_init_ldt_list(void)
{
        int i = 0;

        printk("\nInit init_task ldt table:\n");
        for (i = 0; i < 3; i++)
                printk("0x%x, 0x%x\n", init_task.ldt[i].b, init_task.ldt[i].a);
}

void print_gdt_list(void)
{
	int i = 0;

	printk("\nGdt Table:\n");
	for (i = 1; i <= 3; i++)
		printk("0x%x, 0x%x\n", new_gdt[i].b, new_gdt[i].a);
}

void run_init_task(void)
{
        char c = 'A';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

		x = 2;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t"
                        "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void run_init_task1(void)
{
        char c = 'B';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

		x = 4;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t" "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void run_init_task2(void)
{
        char c = 'C';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

                x = 6;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t" "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void kernel_init(void)
{
	init_vga();
	printk("Init vga ok.\n");

	init_8259A();
	printk("Init 8259A ok.\n");

	init_trap();
	printk("Init trap ok.\n");

	init_mm();
	printk("Init mm ok.\n");

	init_keyboard(); 

	init_schedule();
	init_timer(100);
	printk("Init timer ok.\n");

/*
	printk("0x%x\n", alloc_page(2));
	printk("0x%x\n", alloc_page(4));
	printk("0x%x\n", alloc_page(8));
	printk("0x%x\n", alloc_page(16));

	task_clone1(&task1, (unsigned int)&run_init_task1, 
		(unsigned int)&task1_stack0 + sizeof(task1_stack0), 
		(unsigned int)&task1_stack3 + sizeof(task1_stack3));

        task_clone2((unsigned int)&run_init_task1, 
                (unsigned int)&task1_stack0 + sizeof(task1_stack0), 
                (unsigned int)&task1_stack3 + sizeof(task1_stack3));

        task_clone2((unsigned int)&run_init_task2, 
                (unsigned int)&task2_stack0 + sizeof(task2_stack0), 
                (unsigned int)&task2_stack3 + sizeof(task2_stack3));

        task_clone3((unsigned int)&run_init_task1);
        task_clone3((unsigned int)&run_init_task2);
*/

	sti();
	printk("Start init_task.\n");
	asm("pushl $0x17\n\t"
                "pushl %%eax\n\t"
                "pushfl\n\t"
                "pushl $0x0f\n\t"
                "pushl $1f\n\t"
                "iret\n\t"
                "1:\n\t"
                "movw $0x17, %%ax\n\t"
                "movw %%ax, %%ds\n\t"
                "movw %%ax, %%es\n\t"
                "movw %%ax, %%fs\n\t"
                "movw %%ax, %%gs\n"
                ::"a"(init_task_stack_ring3 + sizeof(init_task_stack_ring3)));

	creat_task((unsigned int)&run_init_task1);
	run_init_task();
}
