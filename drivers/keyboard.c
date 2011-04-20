#include <wos/gdt.h>
#include <wos/asm.h>

#define NULL				(void *)0

extern void keyboard_interrupt();
extern void io_delay(void);
extern int v_scr_x, v_scr_y;

void handle_esc(unsigned char scan_code);
void handle_print(unsigned char scan_code);
void handle_shift(unsigned char scan_code);
void handle_backspace(unsigned char scan_code);
void handle_tab(unsigned char scan_code);
void handle_enter(unsigned char scan_code);
void handle_ctrl_l(unsigned char scan_code);
void handle_shift_l(unsigned char scan_code);
void handle_shift_r(unsigned char scan_code);
void handle_alt_l(unsigned char scan_code);
void handle_caps_lock(unsigned char scan_code);
void handle_F_1_10(unsigned char scan_code);
void handle_num_lock(unsigned char scan_code);
void handle_scroll_lock(unsigned char scan_code);
void handle_home(unsigned char scan_code);
void handle_up(unsigned char scan_code);
void handle_pageup(unsigned char scan_code);
void handle_minus(unsigned char scan_code);
void handle_left(unsigned char scan_code);
void handle_mid(unsigned char scan_code);
void handle_right(unsigned char scan_code);
void handle_plus(unsigned char scan_code);
void handle_end(unsigned char scan_code);
void handle_down(unsigned char scan_code);
void handle_pagedown(unsigned char scan_code);
void handle_ins(unsigned char scan_code);
void handle_dot(unsigned char scan_code);
void handle_f1(unsigned char scan_code);
void handle_f2(unsigned char scan_code);
void handle_f3(unsigned char scan_code);
void handle_f4(unsigned char scan_code);
void handle_f5(unsigned char scan_code);
void handle_f6(unsigned char scan_code);
void handle_f7(unsigned char scan_code);
void handle_f8(unsigned char scan_code);
void handle_f9(unsigned char scan_code);
void handle_f10(unsigned char scan_code);
void handle_f11(unsigned char scan_code);
void handle_f12(unsigned char scan_code);

unsigned char key_buffer[5];
int key_buffer_count = 0;

unsigned int shift_flag = 0;
unsigned int caps_lock_flag = 0;
unsigned int e0_flag = 0;

unsigned char keymap[0x60] = "00"       /* NONE, ESC */
                        "1234567890-="
                        "00"            /* BACKSPACE, TAB */
                        "qwertyuiop[]"
                        "00"             /* ENTER, CTRL_L */
                        "asdfghjkl;\'`"
                        "0"             /* SHIFT_L */
                        "\\zxcvbnm,./"
                        "0"             /* SHIFT_R */
                        "*"
                        "0"             /* ALT_L */
                        " "
                        "0"             /* CAPS_LOCK */
                        "0000000000"    /* F1-F10 */
                        "0"             /* NUM_LOCK */
                        "0"             /* SCROLL_LOCK */
                        "0"             /* PAD_HOME */
                        "0"             /* PAD_UP */
                        "0"             /* PAD_PAGEUP */
                        "0"             /* PAD_MINUS */
                        "0"             /* PAD_LEFT */
                        "0"             /* PAD_MID */
                        "0"             /* PAD_RIGHT */
                        "0"             /* PAD_PLUS */
                        "0"             /* PAD_END */
                        "0"             /* PAD_DOWN */
                        "0"             /* PAD_PAGEDOWN */
                        "0"             /* PAD_INS */
                        "0"             /* PAD_DOT */
                        "000"           /* NONE */
                        "00";           /* F11, F12 */

unsigned char shift_keymap[0x60] = "00"	/* NONE, ESC */
                        "!@#$%^&*()_+"
                        "00"            /* BACKSPACE, TAB */
                        "QWERTYUIOP{}"
                        "00"             /* ENTER, CTRL_L */
                        "ASDFGHJKL:\"~"
                        "0"             /* SHIFT_L */
                        "|ZXCVBNM<>?"
                        "0"             /* SHIFT_R */
                        "*"
                        "0"             /* ALT_L */
                        " "
                        "0"             /* CAPS_LOCK */
                        "0000000000"    /* F1-F10 */
                        "0"             /* NUM_LOCK */
                        "0"             /* SCROLL_LOCK */
                        "7"             /* PAD_HOME */
                        "8"             /* PAD_UP */
                        "9"             /* PAD_PAGEUP */
                        "-"             /* PAD_MINUS */
                        "4"             /* PAD_LEFT */
                        "5"             /* PAD_MID */
                        "6"             /* PAD_RIGHT */
                        "+"             /* PAD_PLUS */
                        "1"             /* PAD_END */
                        "2"             /* PAD_DOWN */
                        "3"             /* PAD_PAGEDOWN */
                        "0"             /* PAD_INS */
                        "0"             /* PAD_DOT */
                        "000"           /* NONE */
                        "00";           /* F11, F12 */

void (*do_handle_code[0x60])(unsigned char) = {NULL, handle_esc, 
				handle_print, handle_print, handle_print, handle_print, handle_print, 
				handle_print, handle_print, handle_print, handle_print, handle_print,
				handle_print, handle_print,	/* 1234567890-= */
				handle_backspace, handle_tab,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
				handle_print, handle_print,	/* qwertyuiop[] */
				handle_enter, handle_ctrl_l,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
                                handle_print, handle_print,     /* asdfghjkl;\` */
				handle_shift_l,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
                                handle_print, handle_print, handle_print, handle_print, handle_print,
                                handle_print, 			/* \zxcvbnm,./ */
				handle_shift_r,
				handle_print, handle_alt_l, handle_print, handle_caps_lock,
				handle_f1, handle_f2, handle_f3, handle_f4, handle_f5,
                                handle_f6, handle_f7, handle_f8, handle_f9, handle_f10,
				handle_num_lock, handle_scroll_lock, handle_home,
				handle_up, handle_pageup, handle_minus, handle_left, handle_mid, handle_right,
				handle_plus, handle_end, handle_down, handle_pagedown, handle_ins,
				handle_dot, NULL, NULL, NULL, handle_f11, handle_f12};
				
void (*do_handle_e0_code[0x60])(unsigned char) = {NULL};
				
void handle_esc(unsigned char scan_code)
{
	printk("[ESC]");
}

void handle_backspace(unsigned char scan_code)
{
	int x;

        if (v_scr_y)
                v_scr_y--;
/*
        set_cursor(v_scr_x, v_scr_y);

	x = (v_scr_x * 80 + v_scr_y) * 2;
        asm("movw $0x18, %%ax\n\t"
                "movw %%ax, %%gs\n\t"
                "movw $0x0, %%ax\n\t"
		"movl %0, %%ebx\n\t"
                "movw %%ax, %%gs:(%%ebx)\n\t"
                ::"m"(x));
*/
}

void handle_tab(unsigned char scan_code)
{
	if (v_scr_y + 8 <= 80) {
		v_scr_y += 8;
	}
	else {
		v_scr_x++;
		if (v_scr_x == 24) {
			v_scr_x = 0;
			screen_clear();
		}
		v_scr_y -= 80;
	}
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_enter(unsigned char scan_code)
{
	v_scr_x++;
	if (v_scr_x == 24) {
		v_scr_x = 0;
		screen_clear();
	}
	v_scr_y = 0;
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_ctrl_l(unsigned char scan_code)
{

}

/* the do_keyboard() function will handle it. */
void handle_shift_l(unsigned char scan_code)
{

}

/* the do_keyboard() function will handle it. */
void handle_shift_r(unsigned char scan_code)
{

}

void handle_alt_l(unsigned char scan_code)
{

}

void handle_caps_lock(unsigned char scan_code)
{
	printk("[CAPS_LOCK]\n");
}

void handle_F_1_10(unsigned char scan_code)
{

}

void handle_num_lock(unsigned char scan_code)
{

}

void handle_scroll_lock(unsigned char scan_code)
{

}

void handle_home(unsigned char scan_code)
{

}
                   
void handle_up(unsigned char scan_code)
{
	if (v_scr_x)
		v_scr_x--;
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_pageup(unsigned char scan_code)
{
	printk("[PAGEUP]");
}

void handle_minus(unsigned char scan_code)
{

}

void handle_left(unsigned char scan_code)
{
	if (v_scr_y)
		v_scr_y--;
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_mid(unsigned char scan_code)
{

}

void handle_right(unsigned char scan_code)
{
	if (v_scr_y < 79)
		v_scr_y++;
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_plus(unsigned char scan_code)
{

}

void handle_end(unsigned char scan_code)
{

}

void handle_down(unsigned char scan_code)
{
	if (v_scr_x < 24)
		v_scr_x++;
	/* set_cursor(v_scr_x, v_scr_y); */
}

void handle_pagedown(unsigned char scan_code)
{
	printk("[PAGEDOWN]");
}

void handle_ins(unsigned char scan_code)
{

}

void handle_dot(unsigned char scan_code)
{

}

void handle_f1(unsigned char scan_code)
{
	printk("[F1]");
}

void handle_f2(unsigned char scan_code)
{

}

void handle_f3(unsigned char scan_code)
{

}

void handle_f4(unsigned char scan_code)
{

}

void handle_f5(unsigned char scan_code)
{

}

void handle_f6(unsigned char scan_code)
{

}

void handle_f7(unsigned char scan_code)
{

}

void handle_f8(unsigned char scan_code)
{

}

void handle_f9(unsigned char scan_code)
{

}

void handle_f10(unsigned char scan_code)
{

}

void handle_f11(unsigned char scan_code)
{

}

void handle_f12(unsigned char scan_code)
{

}

void handle_print(unsigned char scan_code)
{
	if (!caps_lock_flag)
		printk("%c", keymap[scan_code]);
	else
		printk("%c", shift_keymap[scan_code]);
}

void handle_shift(unsigned char scan_code)
{
	printk("%c", shift_keymap[scan_code]);
}

void do_keyboard(void)
{
	unsigned char scan_code;

	scan_code = inb(0x60);

	switch (scan_code) {
	case 0x2a:
	case 0x36:	
		shift_flag = 1;
		break;
	case 0xaa:
	case 0xb6:
		shift_flag = 0;
		break;
	case 0x3a:
		caps_lock_flag = 1;
		break;
	case 0xba:
		caps_lock_flag = 0;
		break;
	case 0xe0:
		e0_flag = 1;
		break;
	default:
		if (scan_code & 0x80)
			return ;

		if (e0_flag) {
			e0_flag = 0;
			(*do_handle_code[scan_code])(scan_code);
			return ;
		}

		if (!shift_flag) {
			(*do_handle_code[scan_code])(scan_code);
		}
		else {
			handle_shift(scan_code);
		}
	}
}

void init_keyboard(void)
{
	set_intr_gate((unsigned int)keyboard_interrupt, 33);
        outb(inb(0x21) & 0xfd, 0x21);
        io_delay();
}
