#ifndef PRINTK_H
#define PRINTK_H

#define va_list                         char*
#define va_start(arg, fortmat)          (arg = (char *)&format + sizeof(format))
#define va_arg(arg, format)             (*(format *)((arg += sizeof(format)) - sizeof(format)))
#define va_end(arg)                     *(char *)arg = 0

#endif
