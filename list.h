#ifndef _LIST_H
#define _LIST_H

typedef  unsigned long _size_t;

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
        list->next = list;
        list->prev = list;
}

static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
        next->prev = new;
        new->next = next;
        new->prev = prev;
        prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
        __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
        __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
        next->prev = prev;
        prev->next = next;
}

static inline void list_del_init(struct list_head *entry)
{
        __list_del(entry->prev, entry->next);
        INIT_LIST_HEAD(entry);
}

static inline int list_empty(const struct list_head *head)
{
        return head->next == head;
}

static inline int list_empty_careful(const struct list_head *head)
{
        struct list_head *next = head->next;
        return (next == head) && (next == head->prev);
}

static inline void list_replace(struct list_head *old,
                                struct list_head *new)
{
        new->next = old->next;
        new->next->prev = new;
        new->prev = old->prev;
        new->prev->next = new;
}

static inline void list_replace_init(struct list_head *old,
                                        struct list_head *new)
{
        list_replace(old, new);
        INIT_LIST_HEAD(old);
}

#define offsetof(TYPE, MEMBER) ((_size_t) &((TYPE *)0)->MEMBER)

/**
 *  * container_of - cast a member of a structure out to the containing structure
 *   * @ptr:        the pointer to the member.
 *    * @type:       the type of the container struct this is embedded in.
 *     * @member:     the name of the member within the struct.
 *      *
 *       */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 *  * list_entry - get the struct for this entry
 *   * @ptr:        the &struct list_head pointer.
 *    * @type:       the type of the struct this is embedded in.
 *     * @member:     the name of the list_struct within the struct.
 *      */
#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/**
 *  * list_for_each_entry  -       iterate over list of given type
 *   * @pos:        the type * to use as a loop cursor.
 *    * @head:       the head for your list.
 *     * @member:     the name of the list_struct within the struct.
 *      */
#define list_for_each_entry(pos, head, member)                          \
        for (pos = list_entry((head)->next, typeof(*pos), member);      \
             &pos->member != (head);    \
             pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 *  * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 *   * @pos:    the type * to use as a loop cursor.
 *    * @n:      another type * to use as temporary storage
 *     * @head:   the head for your list.
 *      * @member: the name of the list_struct within the struct.
 *       */
#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_entry((head)->next, typeof(*pos), member),  \
        n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif
