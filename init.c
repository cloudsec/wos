#include "gdt.h"
#include "asm.h"
#include "task.h"
#include "mm.h"
#include "string.h"

extern struct task_struct init_task;
extern void default_int(void);
extern void run_init_task(void);

asm(".align 16\n");
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

void kernel_init(void)
{
	init_vga();
	init_8259A();
	init_mm();
	init_trap();
	/* the keyboard has some bug may genterate an GP protection. */
	//init_keyboard();
	init_schedule();
	init_timer(100);

	sti();
	printk("Loading Kernel Into Protect Mode OK.\n");
	print_gdt_list();
	printk("Move to ring3, start init task.\n");
        asm("pushfl\n\t"
                "andl $0xffffbfff, (%%esp)\n\t"
                "popfl\n\t"
		"movl %%esp, %%eax\n\t"
                "pushl $0x17\n\t"
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
                :::"ax");
	run_init_task();
	for (;;);
}
