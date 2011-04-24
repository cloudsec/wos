#include <wos/mm.h>

void do_page_fault(unsigned int esp)
{
        unsigned int cr2;

        asm("movl %%cr2, %%eax\n":"=a"(cr2));
        printk("Page fault at: 0x%x\n", cr2);

        panic("page_fault", esp);
}
