#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/vga.h>

void __put_c(char c, int x)
{
        asm("movw $0x18, %%ax\n\t"
                "movw %%ax, %%gs\n\t"
                "movb $0x07, %%ah\n\t"
                "movb %0, %%al\n\t"
                "movl %1, %%edi\n\t"
                "movw %%ax, %%gs:(%%edi)\n\t"
                ::"m"(c),"m"(x));
}

void put_c(char c)
{
        int scr_pos;

	switch (c) {
	case '\n':
		v_scr_x++;
		if (v_scr_x == 24) {
			v_scr_x = 23;
			//screen_clear();
			screen_roll();
		}
		v_scr_y = 0;
		set_cursor(v_scr_x, v_scr_y);
		break;
	case '\r':
		v_scr_y = 0;
		set_cursor(v_scr_x, v_scr_y);
		break;
	case '\t':
		if (v_scr_y + 8 <= 80) {
			v_scr_y += 8;
		}
		else {
			v_scr_x++;
			if (v_scr_x == 24) {
				v_scr_x = 23;
				//screen_clear();
				screen_roll();
			}
			v_scr_y -= 80;
		}
		set_cursor(v_scr_x, v_scr_y);
		break;
	default:
		scr_pos =  (v_scr_x * 80 + v_scr_y) * 2;
        	__put_c(c, scr_pos);

		if (v_scr_y > 80) {
			v_scr_x++;
			v_scr_y = 0;
			if (v_scr_x == 24) {
				v_scr_x = 23;
				//screen_clear();
				screen_roll();
			}
		}
		else {
			v_scr_y++;
		}

		set_cursor(v_scr_x, v_scr_y);
		break;
	}
}

void put_s(char *string)
{
        char *s = string;

        if (!string)
                return ;

        while (*s) {
                put_c(*s);
                s++;
        }
}

void set_cursor(int x, int y)
{
	/* cli(); */
	outb(0x0e, 0x3d4);
	outb(((x * 80 + y) >> 8) & 0xff, 0x3d5);
	outb(0x0f, 0x3d4);
	outb((x * 80 + y) & 0xff, 0x3d5);	
	/* sti(); */
}

void __screen_roll(int src, int dst)
{
	asm("movl $0x18, %%eax\n\t"
		"movw %%ax, %%gs\n\t"
		"movl %0, %%esi\n\t"
		"movl %1, %%edi\n\t"
		"movl $80, %%ecx\n\t"
		"1:\n\t"
		"movw %%gs:(%%esi), %%ax\n\t"
		"movw %%ax, %%gs:(%%edi)\n\t"
		"addl $2, %%esi\n\t"
		"addl $2, %%edi\n\t"
		"decl %%ecx\n\t"
		"test %%ecx, %%ecx\n\t"
		"jne 1b\n"
		::"m"(src), "m"(dst));
}

void screen_roll(void)
{
	int src, dst;
	int i;

	for (i = 1; i <= 24; i++) {
		src = i * 80 * 2;
		dst = (i - 1) * 80 * 2;
		__screen_roll(src, dst);
	}	

/*
        for (i = 0; i + 2 < 80; i += 2)
                __put_c(0x0, 23 * 80 * 2 + i); 
*/
	set_cursor(24, 0);
}

void screen_clear(void)
{
        asm("movw $0x18, %%ax\n\t"
                "movw %%ax, %%gs\n\t"
                "movw $0x0, %%ax\n\t"
                "xorl %%ebx, %%ebx\n\t"
                "1:\n\t"
                "movw %%ax, %%gs:(%%bx)\n\t"
                "addl $2, %%ebx\n\t"
                "cmpl $4000, %%ebx\n\t"
                "jb 1b\n"::);

        set_cursor(0, 0);
}

void init_vga(void)
{
        v_scr_x = v_scr_y = 0;
}
