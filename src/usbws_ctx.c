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

#include <libwebsockets.h>
#include <signal.h>
#include "usbws_ctx.h"

void usbws_set_info(struct lws_context_creation_info *info, void *user,
		    int port, int ssl, const char *key, const char *cert)
{
	memset(info, 0, sizeof(struct lws_context_creation_info));
	info->protocols = usbws_protocols;
	info->port = port;
	if (ssl) {
		info->ssl_private_key_filepath = key;
		info->ssl_cert_filepath = cert;
	}
	info->user = user;
}

int usbws_health_check(struct lws_context *context)
{
	return lws_callback_all_protocol(context, usbws_protocols,
					 USBWS_CALLBACK_HEALTH_CHECK);
}

int usbws_request_send(struct lws_context *context)
{
	return lws_callback_all_protocol(context, usbws_protocols,
					 USBWS_CALLBACK_SEND_REQUEST);
}

static struct usbws_ctx *servicing_ctx;

static void usbws_sighandler(int sig)
{
	usbip_break_all_connections();
	if (servicing_ctx)
		usbws_ctx_stop(servicing_ctx);
}

int usbws_set_sigint(struct usbws_ctx *ctx)
{
	servicing_ctx = ctx;
	signal(SIGINT, usbws_sighandler);
}
