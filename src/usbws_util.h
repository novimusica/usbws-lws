/*
 * Copyright (C) 2016 Nobuo Iwata
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __USBWS_UTIL_H
#define __USBWS_UTIL_H

#if defined(_WIN32)
#include "usbws_win32.h"
#else
#include <pthread.h>
#endif

#ifndef usbws_cond_lock_t
#define usbws_cond_lock_t pthread_mutex_t
#endif
#ifndef usbws_cond_lock_init
#define usbws_cond_lock_init(lock, atr) pthread_mutex_init(lock, atr)
#endif
#ifndef usbws_cond_lock
#define usbws_cond_lock(lock) pthread_mutex_lock(lock)
#endif
#ifndef usbws_cond_unlock
#define usbws_cond_unlock(lock) pthread_mutex_unlock(lock)
#endif

#if defined(__unix__)
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

#if defined(__unix__)
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif

#if defined(__unix__)
#define container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member)*__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); })
#endif

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *neo,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = neo;
	neo->next = next;
	neo->prev = prev;
	prev->next = neo;
}

static inline void list_add_tail(struct list_head *neo, struct list_head *head)
{
	 __list_add(neo, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

int usbws_get_port(int port, int ssl);
void usbws_version(void);
void usbws_set_debug(int opt_debug);

#endif /* !__USBWS_UTIL_H */
