#ifndef STRING_H
#define STRING_H

char *itoa(int num, char *str, int radix);
int strlen(char *str);
char *strcpy(char *dst, char *src);
char *strncpy(char *dst, char *src, int n);
int strcmp(char *src, char *dst);
int strncmp(char *src, char *dst, int n);
void *memcpy(void *dst, void *src, int n);
void *memset(void *s, int c, int n);

#endif
