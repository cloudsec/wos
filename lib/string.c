#include <wos/string.h>
#include <wos/type.h>

char *itoa(int num, char *str, int radix)
{
        char string[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        char *s = str, *p;
        char tmp;

        if (num < 0)
                return NULL;

        if (!num) {
                *s++ = '\0';
                *s = 0;
                return s;
        }

        while (num) {
                *s++ = string[num % radix];
                num /= radix;
        }
        *s = 0;

        --s;
        for (p = str; p < s; s--, p++) {
                tmp = *p;
                *p = *s;
                *s = tmp;
        }

        return s;
}

int strlen(char *str)
{
	char *s = str;
	
	if (!s)
		return -1;

	while (*s++);

	return (int)(s - str - 1);
}

char *strcpy(char *dst, char *src)
{
	char *s = dst, *p = src;

	while (*p)
		*s++ = *p++;
	*s = 0;

	return dst;
}

char *strncpy(char *dst, char *src, int n)
{
	char *s = src, *p = dst;
	int i = 0;

	while (*s && i < n) {
		*p++ = *s++;
		i++;
	}
	*p = 0;

	return dst;
}

int strcmp(char *src, char *dst)
{
	char *s = src, *p = dst;

	while (*s == *p) {
		s++;p++;
	}

	if (*s == *p)
		return 0;
	if (*s > *p)
		return 1;
	if (*s < *p)
		return -1;
}

int strncmp(char *src, char *dst, int n)
{
        char *s = src, *p = dst;
	int i = 0;

        while ((*s == *p) && i < n) {
                s++;p++;i++;
        }

        if (*s == *p)
                return 0;
        if (*s > *p)
                return 1;
        if (*s < *p)
                return -1;
}

void *memcpy(void *dst, void *src, int n)
{
        char *s = src, *p = dst;
        int i = 0;

        while (i < n) {
                *p++ = *s++;
                i++;
        }
}

void *memset(void *s, int c, int n)
{
	int i;
	char *p = s;

	for (i = 0; i < n; i++)
		*p++ = c;
}
