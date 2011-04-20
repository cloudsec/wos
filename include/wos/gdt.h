#ifndef GDT_H
#define GDT_H

#define KERNEL_CODE_TYPE		0x9a
#define KERNEL_DATA_TYPE		0x92
#define VIDEO_TYPE			0xf2

#define USER_CODE_TYPE			0xfa
#define USER_DATA_TYPE			0xf2

#define LDT_TYPE			0x82
#define TSS_TYPE			0x89

#define DESC_DEFAULT_TYPE       	0xc
#define DESC_SYSTEM_TYPE		0x0

#define KERNEL_CODE_IDX			0x1
#define KERNEL_DATA_IDX			0x2
#define VIDEO_IDX			0x3

#define CODE_LIMIT              	0xffffffff
#define DATA_LIMIT              	CODE_LIMIT
#define USER_CODE_LIMIT			0xffff
#define USER_DATA_LIMIT			0xffff
#define TSS_LIMIT			0x68
#define LDT_LIMIT			0x40

#define CODE_BASE               	0x0
#define DATA_BASE               	CODE_BASE

#define VIDEO_LIMIT             	0xffff
#define VIDEO_BASE              	0xb8000

#define KERNEL_CODE_SEL			0x08	/*  01000 */
#define KERNEL_DATA_SEL			0x10	/*  10000 */
#define VIDEO_SEL			0x18	/*  11000 */

#define USER_CODE_SEL			0x0f	/*  1111 */
#define USER_DATA_SEL			0x17	/*  1111 */

#define IDT_SYSTEM_TYPE                 0xef00
#define IDT_INTR_TYPE                   0x8e00
#define IDT_TRAP_TYPE                   0x8f00

#define set_intr_gate(base, idx)        set_idt_desc(base, KERNEL_CODE_SEL,     \
                                                IDT_INTR_TYPE, idx)
#define set_trap_gate(base, idx)        set_idt_desc(base, KERNEL_CODE_SEL,     \
                                                IDT_TRAP_TYPE, idx)
#define set_system_gate(base, idx)      set_idt_desc(base, KERNEL_CODE_SEL,     \
                                                IDT_SYSTEM_TYPE, idx)

struct gdt_desc {
        unsigned int a;
        unsigned int b;
};

void __set_gdt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit,
                int type_h, int type, int idx);
void set_gdt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit,
                int type, int idx);
void set_ldt_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit,
                int type, int idx);
void set_tss_desc(struct gdt_desc *addr, unsigned int base, unsigned int limit,
                int type, int idx);

#endif
