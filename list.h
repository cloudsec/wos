#ifndef LIST_H
#define LIST_H

struct list_head {
	struct list_head *prev, *next;
};

#define INIT_LIST_HEAD(name_ptr)	do { 	(name_ptr)->next = (name_ptr);	\
						(name_ptr)->prev = (name_ptr);	\
					}while (0)

#define OFFSET(type, member)            (char *)&(((type *)0x0)->member)
#define container_of(ptr, type, member) ({					\
			(type *)((char * )ptr - OFFSET(type, member)); });
	
#define list_for_each(pos, head)        for (pos = head->next; pos != head; pos = pos->next)
#define list_for_each_prev(pos, head)	for (pos = head->prev; pos != head; pos = pos->prev)
#define list_entry(ptr, type, member)   container_of(ptr, type, member)

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	head->prev->next = new;
	new->prev = head->prev;
	new->next = head;
	head->prev = new;
}

static inline void list_del(struct list_head *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
}

static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

#endif
