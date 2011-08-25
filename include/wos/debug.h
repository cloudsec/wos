#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG		0

#if DEBUG == 1
#define DbgPrint	printk
#else
#define DbgPrint	
#endif

#endif
