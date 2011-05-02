#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/hd.h>

extern void hd_interrupt(void);

struct hd_param {
	unsigned int cyl;
	unsigned int head;
	unsigned int sect;
}HD = {203, 16, 63};

void hd_ready_status(void)
{
	unsigned char status = -1;

	for (;;) {
		status = inb(HD_PORT_STATUS);
		printk("0x%x\n", status);
		if (status & 0x40) {
			printk("hard disk is ready.\n");
			break;
		}
	}
}

void hd_transfer_status(void)
{
        unsigned char status = -1;

        for (;;) {
                status = inb(HD_PORT_STATUS);
                printk("0x%x\n", status);
                if (status & 0x08) {
                        printk("hard disk is ready to transfer datas.\n");
                        break;
                }
        }
}

int hd_read(unsigned int lba, unsigned int sects, void *buf)
{
	unsigned int cyl = lba / (HD.head * HD.sect);
	unsigned int head = (lba % (HD.head * HD.sect)) / HD.sect;
	unsigned int sect = (lba % (HD.head * HD.sect)) % HD.sect + 1;

	hd_ready_status();
	
	outb(sects, HD_PORT_SECT_COUNT);
	outb(sect, HD_PORT_SECT_NUM);
	outb(cyl, HD_PORT_CYL_LOW);
	outb(cyl >> 8, HD_PORT_CYL_HIGH);
	outb(0xa0 | head, HD_PORT_DRV_HEAD);
	outb(HD_READ, HD_PORT_COMMAND);

	hd_transfer_status();

	insl(HD_PORT_DATA, buf, sects << 7);
}

int hd_write(unsigned int lba, unsigned int sects, void *buf)
{
	unsigned int cyl = lba / (HD.head * HD.sect);
	unsigned int head = (lba % (HD.head * HD.sect)) / HD.sect;
	unsigned int sect = (lba % (HD.head * HD.sect)) % HD.sect + 1;

	hd_ready_status();
	
	outb(sects, HD_PORT_SECT_COUNT);
	outb(sect, HD_PORT_SECT_NUM);
	outb(cyl, HD_PORT_CYL_LOW);
	outb(cyl >> 8, HD_PORT_CYL_HIGH);
	outb(0xa0 | head, HD_PORT_DRV_HEAD);
	outb(HD_WRITE, HD_PORT_COMMAND);

	hd_transfer_status();

	outsl(buf, sects << 7, HD_PORT_DATA);
}

void hd_test(void)
{
	unsigned char sect[512] = {0};
	
	hd_read(0, 1, sect);

	printk("!0x%x, 0x%x\n", sect[0x1fe], sect[0x1ff]);
}

void setup_dpt(void)
{
	unsigned char sect[512] = {0};
	
	sect[0x1be] = 0x80;
	sect[0x1be + 0x04] = 0x85;
	*(unsigned long *)&sect[0x1be + 0x08] = 1;
	*(unsigned long *)&sect[0x1be + 0x0c] = 85*1024*2;

	sect[0x1fe] = 0x55;
	sect[0x1ff] = 0xaa;

	hd_write(0, 1, sect);
}

int do_hd(void)
{
	printk("hd interrupt.\n");

	return 0;
}

void init_hd(void)
{
	HD.cyl = 203; HD.head = 16, HD.sect = 63;

	set_intr_gate(hd_interrupt, 0x2E);
	outb(inb(0x21)&0xfb, 0x21);
	outb(inb(0xa1)&0xbf, 0xa1);
}
