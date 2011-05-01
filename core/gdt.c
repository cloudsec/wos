#include <wos/gdt.h>
#include <wos/task.h>

extern struct task_struct init_task;

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
