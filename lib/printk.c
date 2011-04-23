#include <wos/printk.h>
#include <wos/string.h>

extern int v_scr_x, v_scr_y;

int vfprintf(char *format, va_list arg)
{
	int flag = 0, ret = 0;
	const char *p = format;

	while (*p) {
		switch (*p) {
		case '%':
			if (flag) {
				flag = 0;
				put_c(*p);
				ret++;
			}
			else {
				flag = 1;
			}
			break;
		case 'd':
			if (flag) {
				char buf[16];
				flag = 0;

				itoa(va_arg(arg, int), buf, 10);
				put_s(buf);
				ret += strlen(buf);
			}
			else {
				put_c(*p);
				ret++;
			}
			break;
                case 'x':
                        if (flag) {
                                char buf[64];
                                flag = 0;

                                itoa(va_arg(arg, int), buf, 16);
                                put_s(buf);
                                ret += strlen(buf);
                        }
                        else {
                                put_c(*p);
                                ret++;
                        }
                        break;
		case 'b':
                        if (flag) {
                                char buf[16];
                                flag = 0;

                                itoa(va_arg(arg, int), buf, 2);
                                put_s(buf);
                                ret += strlen(buf);
                        }
                        else {
                                put_c(*p);
                                ret++;
                        }
                        break;
		case 's':
			if (flag) {
				char *str = va_arg(arg, char*);
				flag = 0;
				
				put_s(str);
				ret += strlen(str);
			}
			else {
				put_c(*p);
				ret++;
			}
			break;
                case 'c':
                        if (flag) {
                                char s = va_arg(arg, char);
                                flag = 0;

                                put_c(s);
				ret++;
                        }
                        else {
                                put_c(*p);
                                ret++;
                        }
                        break;
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
			put_c(*p);
			ret++;
			break;
		}
		*p++;
	}

	va_end(arg);
	return ret;
}

int printk(char *format, ...)
{
	va_list arg;
	va_start(arg, format);

	return vfprintf(format, arg);
}
