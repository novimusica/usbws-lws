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

#ifndef __USBWS_CTX_H
#define __USBWS_CTX_H

#include <libwebsockets.h>
#include "usbws_util.h"

#define USBWS_PING_PONG_DEFAULT 60
#define USBWS_PING_PONG_TIMEOUT 60
#define USBWS_PING_PONG_CLIENT_MARGIN 60

struct usbws_ctx {
	int cont;
	int ping_pong;
	struct lws_context *context;
	int (*start)(struct lws *wsi);
	int (*stop)(struct lws *wsi);
};

static inline struct usbws_ctx *context2ctx(struct lws_context *context)
{
	return (struct usbws_ctx *)lws_context_user(context);
}

static inline struct lws_context *ctx2context(struct usbws_ctx *ctx)
{
	return ctx->context;
}

static inline int usbws_ctx_init(struct usbws_ctx *ctx,
			     int (*start)(struct lws *wsi),
			     int (*stop)(struct lws *wsi))
{
	ctx->cont = 1;
	ctx->ping_pong = USBWS_PING_PONG_DEFAULT;
	ctx->start = start;
	ctx->stop = stop;
}

static inline struct lws_context *
usbws_ctx_create(struct usbws_ctx *ctx, struct lws_context_creation_info *info)
{
	ctx->context = lws_create_context(info);
	return ctx->context;
}

static inline void usbws_ctx_destroy(struct usbws_ctx *ctx)
{
	if (ctx->context) {
		lws_context_destroy(ctx->context);
		ctx->context = NULL;
	}
}

static inline void usbws_ctx_set_ping_pong(struct usbws_ctx *ctx,
					   int ping_pong)
{
	ctx->ping_pong = ping_pong;
}

static inline int usbws_ctx_get_ping_pong(struct usbws_ctx *ctx)
{
	return ctx->ping_pong;
}

static inline void usbws_ctx_stop(struct usbws_ctx *ctx)
{
	ctx->cont = 0;
	lws_cancel_service(ctx->context);
}

static inline int usbws_ctx_stopped(struct usbws_ctx *ctx)
{
	return !ctx->cont;
}

enum usbwsd_callback_reasons {
	USBWS_CALLBACK_HEALTH_CHECK = LWS_CALLBACK_USER,
	USBWS_CALLBACK_SEND_REQUEST
};

extern const struct lws_protocols usbws_protocols[];

void usbws_set_info(struct lws_context_creation_info *info, void *data,
		    int port, int ssl, const char *key, const char *cert);
int usbws_health_check(struct lws_context *context);
int usbws_request_send(struct lws_context *context);
int usbws_set_sigint(struct usbws_ctx *ctx);

#endif /* !__USBWS_CTX_H */
