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

#ifndef __USBWS_CLIENT_H
#define __USBWS_CLIENT_H

#include "usbws_util.h"
#include "usbws_ctx.h"
#include "usbws_session.h"

extern const char *usbws_default_key;
extern const char *usbws_default_cert;

#define USBWS_TCP_PORT_LEN 7

struct usbws_client {
	struct usbws_ctx ctx;
	const char *url;
	char *url_work;
	char *proxy;
	const char *method;
	const char *host;
	int tcp_port;
	const char *path;
	const char *proxy_host;
	int proxy_port;
	char tcp_port_s[USBWS_TCP_PORT_LEN + 1];
	char ssl;
	char verification;
	const char *key;
	const char *cert;
	const char *root_cert;
	pthread_t tid;
	char started;
	usbws_cond_lock_t lock;
	pthread_cond_t cond;
	struct lws *wsi;
};

#define USBWS_VERIFY_NONE	0
#define USBWS_VERIFY_RELAXED	1

static inline struct usbws_ctx *client2ctx(struct usbws_client *client)
{
	return &client->ctx;
}

static inline struct usbws_client *ctx2client(struct usbws_ctx *ctx)
{
	return (struct usbws_client *)container_of(ctx, struct usbws_client,
						   ctx);
}

void usbws_client_init(struct usbws_client *client);
int usbws_client_handle_verification(const char *arg,
				     struct usbws_client *client);
void usbws_client_free(struct usbws_client *client);
int usbws_client_set_target(struct usbws_client *client,
			    const char *url, const char *proxy);
int usbws_client_set_ping_pong(struct usbws_client *client, int ping_pong);

#endif /* !__USBWS_CLIENT_H */
