#ifndef VGA_H
#define VGA_H

int v_scr_x, v_scr_y;

void set_cursor(int x, int y);
void put_s(char *string);
void screen_roll(void);
void screen_clear(void);

#endif
