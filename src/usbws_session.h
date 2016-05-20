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

#ifndef __USBWS_SESSION_H
#define __USBWS_SESSION_H

#include <time.h>
#include <libwebsockets.h>
#include <usbip_api.h>
#include "usbws_util.h"

struct usbws_session {
	char cont;
	char writable;
	char pinged;
	char ping_pending;
	time_t stamp;
	pthread_mutex_t writable_lock;
	usbws_cond_lock_t send_complete_lock;
	pthread_cond_t send_complete_cond;
	void *send_buf;
	int send_len;
	int send_offset;
	usbws_cond_lock_t recv_queue_lock;
	pthread_cond_t recv_queue_cond;
	struct list_head recv_queue;
	int recv_offset;
	pthread_t tid;
};

struct usbws_recv_buf {
	struct list_head list;
	int len;
	char buf[];
};

static inline struct usbws_session *wsi2session(struct lws *wsi)
{
	return (struct usbws_session *)lws_wsi_user(wsi);
}

void usbws_session_set_service(struct usbws_session *session,
				int (*established)(struct lws *wsi),
				int (*destroyed)(struct lws *wsi));

void usbws_session_discontinue(struct lws *wsi);

void usbws_sock_init(struct usbip_sock *sock, struct lws *wsi);
const char *usbws_protocol_name(void);

#endif /* !__USBWS_SESSION_H */
