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
#include <usbip_api.h>
#include "usbws_client.h"

const char *usbws_default_key = "cert/server.key";
const char *usbws_default_cert = "cert/server.crt";

static void *usbws_client(void *arg)
{
	struct usbws_client *client = (struct usbws_client *)arg;
	struct usbws_ctx *ctx = client2ctx(client);
	struct lws_context *context = ctx2context(ctx);
	int timeout = (usbws_ctx_get_ping_pong(ctx) +
		       USBWS_PING_PONG_CLIENT_MARGIN) * 1000;

	while (!usbws_ctx_stopped(ctx)) {
		if (lws_service(context, timeout))
			break;
	}
	if (client->wsi)
		usbws_session_discontinue(client->wsi);
	usbws_ctx_destroy(ctx);
	lwsl_debug("end of client thread\n");
	return NULL;
}

static inline int usbws_client_get_ssl(struct usbws_client *client)
{
	if (client->ssl) {
		if (client->verification == USBWS_VERIFY_RELAXED)
			return 1;
		else
			return 2;
	}
	return 0;
}

static void usbws_client_conn_init(struct usbws_client *client,
				   struct lws_context *context,
				   struct lws_client_connect_info *conn)
{
	memset(conn, 0, sizeof(struct lws_client_connect_info));
	conn->context = context;
	conn->ssl_connection = usbws_client_get_ssl(client);
	conn->address = client->host;
	conn->host = client->host;
	conn->origin = client->host;
	conn->port = client->tcp_port;
	conn->path = client->url;
	conn->protocol = usbws_protocol_name();
	conn->ietf_version_or_minus_one = -1;
}

static int usbws_client_wait_start(struct usbws_client *client)
{
	lwsl_debug("waiting client start\n");
	usbws_cond_lock(&client->lock);
	while (!client->started)
		pthread_cond_wait(&client->cond, &client->lock);
	usbws_cond_unlock(&client->lock);
	lwsl_debug("started client %d\n", client->started);
	if (client->started < 0)
		return -1;
	return 0;
}

static void __notify_start(struct usbws_client *client, int val)
{
	usbws_cond_lock(&client->lock);
	client->started = val;
	pthread_cond_signal(&client->cond);
	usbws_cond_unlock(&client->lock);
}

static inline void usbws_client_notify_start(struct usbws_client *client)
{
	lwsl_debug("resuming client ok\n");
	__notify_start(client, 1);
}

static inline void usbws_client_notify_error(struct usbws_client *client)
{
	lwsl_debug("resuming client error\n");
	__notify_start(client, -1);
}

static struct usbip_sock *usbws_client_open(const char *host, const char *port,
					    void *opt)
{
	struct usbws_client *client = (struct usbws_client *)opt;
	struct usbws_ctx *ctx = client2ctx(client);
	struct lws_context_creation_info info;
	struct lws_client_connect_info conn;
	struct lws_context *context;
	int ssl;
	struct lws *wsi;
	struct usbip_sock *sock;
	int client_created = 0;

	usbws_set_info(&info, &client->ctx,
		       CONTEXT_PORT_NO_LISTEN, client->ssl,
		       client->key, client->cert);

	context = usbws_ctx_create(ctx, &info);
	if (!context) {
		lwsl_err("failed to create context\n");
		goto err_out;
	}
	if (client->proxy && lws_set_proxy(context, client->proxy)) {
		lwsl_err("failed to set proxy\n");
		goto err_destroy_context;
	}

	usbws_client_conn_init(client, context, &conn);

	wsi = lws_client_connect_via_info(&conn);
	if (!wsi) {
		lwsl_err("failed to connect %d://%s:%s/%s", ssl, host, port);
		goto err_destroy_context;
	}
	sock = (struct usbip_sock *)calloc(1, sizeof(struct usbip_sock));
	if (!sock) {
		lwsl_err("failed to alloc sock\n");
		goto err_destroy_context;
	}
	usbws_sock_init(sock, wsi);

	usbws_set_sigint(ctx);

	if (pthread_create(&client->tid, NULL, usbws_client, client)) {
		lwsl_err("failed to create client\n");
		goto err_destroy_context;
	}
	client_created = 1;

	if (usbws_client_wait_start(client)) {
		lwsl_err("failed to wait start\n");
		goto err_free_sock;
	}

	return sock;

err_free_sock:
	free(sock);
err_destroy_context:
	if (!client_created)
		usbws_ctx_destroy(ctx);
err_out:
	return NULL;
}

static void usbws_client_close(struct usbip_sock *sock)
{

	free(sock);
	lwsl_debug("client closed\n");
}

static int usbws_client_start_session(struct lws *wsi)
{
	struct lws_context *context = lws_get_context(wsi);
	struct usbws_client *client = ctx2client(context2ctx(context));

	client->wsi = wsi;
	usbws_client_notify_start(client);
}

static int usbws_client_stop_session(struct lws *wsi)
{
	struct lws_context *context = lws_get_context(wsi);
	struct usbws_ctx *ctx = context2ctx(context);
	struct usbws_client *client = ctx2client(ctx);

	client->wsi = NULL;
	usbws_ctx_stop(ctx);
	usbws_client_notify_error(client);
}

void usbws_client_init(struct usbws_client *client)
{
	struct usbws_ctx *ctx = client2ctx(client);

	memset(client, 0, sizeof(struct usbws_client));
	usbws_ctx_init(ctx, usbws_client_start_session,
			    usbws_client_stop_session);
	client->key = usbws_default_key;
	client->cert = usbws_default_cert;
	client->verification = USBWS_VERIFY_NONE;
	usbws_cond_lock_init(&client->lock, NULL);
	pthread_cond_init(&client->cond, NULL);
	usbip_conn_init(usbws_client_open, usbws_client_close, client);
}

int usbws_client_handle_verification(const char *arg,
				     struct usbws_client *client)
{
	if (strcmp(arg, "none") == 0) {
		client->verification = USBWS_VERIFY_NONE;
		return 0;
	} else if (strcmp(arg, "relaxed") == 0) {
		client->verification = USBWS_VERIFY_RELAXED;
		return 0;
	}
	lwsl_err("invalid verification %s", arg);
	return -1;
}

void usbws_client_free(struct usbws_client *client)
{
	if (client->url_work)
		free(client->url_work);
	if (client->proxy)
		free(client->proxy);
}

static int usbws_client_set_proxy(struct usbws_client *client,
				   const char *proxy)
{
	char *tmp;
	const char *method, *host, *path;
	int port, len;

	if (!proxy) {
		client->proxy = NULL;
		return 0;
	}

	tmp = (char *)malloc(strlen(proxy + 1));
	if (!tmp) {
		lwsl_err("failed to alloc proxy work\n");
		goto err_out;
	}
	strcpy(tmp, proxy);

	if (lws_parse_uri(tmp, &method, &host, &port, &path)) {
		lwsl_err("failed to parse proxy\n");
		goto err_free;
	}

	len = strlen(host) + USBWS_TCP_PORT_LEN + 2;
	client->proxy = (char *)malloc(len);
	if (!client->proxy) {
		lwsl_err("failed to alloc proxy\n");
		goto err_free;
	}
	snprintf(client->proxy, len, "%s:%d", host, port);

	return 0;
err_free:
	free(tmp);
err_out:
	return -1;
}

int usbws_client_set_target(struct usbws_client *client,
			    const char *url, const char *proxy)
{
	client->url = url;
	client->url_work = (char *)malloc(strlen(url) + 1);
	if (!client->url_work) {
		lwsl_err("failed to alloc url\n");
		goto err_out;
	}
	strcpy(client->url_work, url);

	if (lws_parse_uri(client->url_work, &client->method,
			  &client->host, &client->tcp_port, &client->path)) {
		lwsl_err("failed to parse url\n");
		goto err_free;
	}
	snprintf(client->tcp_port_s, USBWS_TCP_PORT_LEN, "%d",
				     client->tcp_port);

	if (strcmp(client->method, "ws") == 0)
		client->ssl = 0;
	else if (strcmp(client->method, "wss") == 0)
		client->ssl = 1;
	else {
		lwsl_err("unsupported method in url: %s\n", client->method);
		goto err_free;
	}

	if (usbws_client_set_proxy(client, proxy))
		goto err_free;

	return 0;
err_free:
	free(client->url_work);
	client->url = NULL;
err_out:
	return -1;
}
