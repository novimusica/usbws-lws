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
#include "usbws_ctx.h"
#include "usbws_session.h"

static void usbws_session_init(struct usbws_session *session)
{
	memset(session, 0, sizeof(struct usbws_session));
	session->cont = 1;
	pthread_mutex_init(&session->writable_lock, NULL);
	usbws_cond_lock_init(&session->send_complete_lock, NULL);
	pthread_cond_init(&session->send_complete_cond, NULL);
	usbws_cond_lock_init(&session->recv_queue_lock, NULL);
	pthread_cond_init(&session->recv_queue_cond, NULL);
	INIT_LIST_HEAD(&session->recv_queue);
	session->stamp = time(NULL);
}

void usbws_session_discontinue(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	struct lws_context *context = lws_get_context(wsi);

	lwsl_debug("discontinue %p\n", wsi);

	session->cont = 0;

	usbws_cond_lock(&session->send_complete_lock);
	pthread_cond_signal(&session->send_complete_cond);
	usbws_cond_unlock(&session->send_complete_lock);

	usbws_cond_lock(&session->recv_queue_lock);
	pthread_cond_signal(&session->recv_queue_cond);
	usbws_cond_unlock(&session->recv_queue_lock);
}

static void usbws_wait_recv(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	int queued = 0;
	int retry = 1;

	while (session->cont && retry > 0) {
		usbws_cond_lock(&session->recv_queue_lock);
		if (list_empty(&session->recv_queue))
			queued = 0;
		else
			queued = 1;
		usbws_cond_unlock(&session->recv_queue_lock);
		if (!queued)
			break;
		lwsl_debug("waiting recv\n");
		sleep(1);
		--retry;
	}
	if (queued)
		lwsl_warn("recv queue not empty at close\n");
}

static void usbws_session_close(struct lws *wsi,
				enum lws_callback_reasons reason)
{
	struct usbws_session *session = wsi2session(wsi);
	struct list_head *p, *n;

	lwsl_debug("closing session %p %d\n", wsi, reason);
	usbws_cond_lock(&session->recv_queue_lock);
	list_for_each_safe(p, n, &session->recv_queue) {
		list_del(p);
	}
	usbws_cond_unlock(&session->recv_queue_lock);
	usbws_session_discontinue(wsi);
	lwsl_debug("closed session %p\n", wsi);
}

int usbws_session_start(struct lws *wsi)
{
	struct lws_context *context = lws_get_context(wsi);
	struct usbws_ctx *ctx = context2ctx(context);

	lwsl_debug("starting session %p\n", wsi);
	if (ctx->start)
		return (*ctx->start)(wsi);
	return 0;
}

int usbws_session_stop(struct lws *wsi)
{
	struct lws_context *context = lws_get_context(wsi);
	struct usbws_ctx *ctx = context2ctx(context);

	lwsl_debug("stopping session %p\n", wsi);
	if (ctx->stop)
		return (*ctx->stop)(wsi);
	return 0;
}

static inline void usbws_session_handled(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);

	session->stamp = time(NULL);
}

static int usbws_handle_recv(struct lws *wsi, void *buf, size_t len)
{
	struct usbws_session *session = wsi2session(wsi);
	struct usbws_recv_buf *recv_buf;

	lwsl_debug("handling recv %p %p(%d)\n", wsi, buf, len);

	if (lws_frame_is_binary(wsi)) {
		recv_buf = (struct usbws_recv_buf *)
				malloc(sizeof(struct usbws_recv_buf) + len);
		if (!recv_buf) {
			lwsl_err("failed to alloc recv buf\n");
			return -1;
		}
		memcpy(recv_buf->buf, buf, len);
		recv_buf->len = len;
		usbws_cond_lock(&session->recv_queue_lock);
		list_add_tail(&recv_buf->list, &session->recv_queue);
		pthread_cond_signal(&session->recv_queue_cond);
		usbws_cond_unlock(&session->recv_queue_lock);
	}
	return 0;
}

#define SEND_CONTENT 1500
#define SEND_BUF_LEN (SEND_CONTENT + LWS_SEND_BUFFER_PRE_PADDING \
				   + LWS_SEND_BUFFER_POST_PADDING)

static int __send_data(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	unsigned char *p, *sbuf = (unsigned char *)session->send_buf;
	int bytes, sent;
	unsigned char buf[SEND_BUF_LEN];

	bytes = session->send_len - session->send_offset;
	bytes = (bytes > SEND_CONTENT) ? SEND_CONTENT : bytes;

	lwsl_debug("sending %p %d bytes\n", wsi, bytes);

	p = buf + LWS_SEND_BUFFER_PRE_PADDING;
	memcpy(p, sbuf + session->send_offset, bytes);
	sent = lws_write(wsi, p, bytes, LWS_WRITE_BINARY);
	session->send_offset += sent;
	if (session->send_offset >= session->send_len) {
		usbws_cond_lock(&session->send_complete_lock);
		session->send_buf = NULL;
		session->send_len = 0;
		pthread_cond_signal(&session->send_complete_cond);
		usbws_cond_unlock(&session->send_complete_lock);
	}
	session->writable = 0;
	lws_callback_on_writable(wsi);
	lwsl_debug("sent %p %d bytes\n", wsi, sent);
	return sent;
}

static int usbws_handle_send_request(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	int ret = 0;

	if (!session->send_buf) /* not for me, go onto next */
		return 0;

	pthread_mutex_lock(&session->writable_lock);
	lwsl_debug("handling send request %p %p(%d) %d\n",
		   wsi, session->send_buf, session->send_len,
		   session->writable);
	if (session->writable)
		ret = __send_data(wsi);
	pthread_mutex_unlock(&session->writable_lock);

	if (!ret) {
		/*
		 * WORKAROUND:
		 * cancel service
		 * otherwise writable will not happen until service timeout.
		 */
		lwsl_debug("canceling service\n");
		lws_cancel_service(lws_get_context(wsi));
	}
	return ret;
}

#define PING_CONTENT 1500
#define PING_BUF_LEN (PING_CONTENT + LWS_SEND_BUFFER_PRE_PADDING \
				   + LWS_SEND_BUFFER_POST_PADDING)

static int __send_ping(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	unsigned char *p;
	int sent;
	unsigned char buf[PING_BUF_LEN];

	lwsl_debug("ping %p\n", wsi);

	p = buf + LWS_SEND_BUFFER_PRE_PADDING;
	*p = '?';
	sent = lws_write(wsi, p, 1, LWS_WRITE_PING);
	if (sent > 0)
		session->pinged = 1;
	else
		lwsl_debug("ping error\n");
	session->writable = 0;
	lws_callback_on_writable(wsi);
	return sent;
}

static int usbws_send_ping(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	unsigned char *p;
	int sent, ret = 0;
	unsigned char buf[SEND_BUF_LEN];

	pthread_mutex_lock(&session->writable_lock);
	if (session->writable)
		ret = __send_ping(wsi);
	if (!ret)
		session->ping_pending = 1;
	pthread_mutex_unlock(&session->writable_lock);
	return ret;
}

static int usbws_handle_writable(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	int ret = 0;

	pthread_mutex_lock(&session->writable_lock);
	lwsl_debug("handling writable %p\n", wsi);
	session->writable = 1;
	if (session->send_buf) {
		session->ping_pending = 0;
		ret = __send_data(wsi);
	} else if (session->ping_pending) {
		session->ping_pending = 0;
		ret = __send_ping(wsi);
	}
	pthread_mutex_unlock(&session->writable_lock);
	return ret;
}

static inline void usbws_session_close_me(struct lws *wsi)
{
	lwsl_debug("close me %p\n", wsi);
	lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, (unsigned char *)"!", 1);
}

static int usbws_check_session(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);
	struct usbws_ctx *ctx = context2ctx(lws_get_context(wsi));
	int delta = difftime(time(NULL), session->stamp);
	int ping_pong = usbws_ctx_get_ping_pong(ctx);

	lwsl_debug("checking session %p %d %d %d/%d\n",
		   wsi, session->cont, session->pinged, delta, ping_pong);
	/*
	 * WORKAROUND:
	 * send ping for closed and closing session
	 * otherwise connection cannot be closed
	 * because CLOSE frame may not be sent.
	 */
	if (!session->cont) {
		usbws_send_ping(wsi);
		usbws_session_close_me(wsi);
		return -1;
	} else if (!ping_pong)
		return 0;
	else if (delta >= (ping_pong + USBWS_PING_PONG_TIMEOUT)) {
		usbws_session_discontinue(wsi);
		usbws_send_ping(wsi);
		usbws_session_close_me(wsi);
		return -1;
	} else if (delta >= ping_pong && !session->pinged)
		usbws_send_ping(wsi);
	return 0;
}

static int usbws_handle_session(struct lws *wsi,
				enum lws_callback_reasons reason,
				void *user, void *in, size_t len)
{
	struct usbws_session *session = wsi2session(wsi);
	int ret = 0;

	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_RECEIVE:
	case LWS_CALLBACK_CLIENT_RECEIVE:
	case LWS_CALLBACK_RECEIVE_PONG:
	case USBWS_CALLBACK_HEALTH_CHECK:
	case USBWS_CALLBACK_SEND_REQUEST:
		if (!session) {
			lwsl_debug("invalid session %p %d\n", wsi, reason);
			return -1;
		}
		break;
	}

	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		usbws_session_init(session);
		lws_callback_on_writable(wsi);
		ret = usbws_session_start(wsi);
		break;
	case LWS_CALLBACK_CLOSED:
		usbws_wait_recv(wsi);
		usbws_session_close(wsi, reason);
		ret = usbws_session_stop(wsi);
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		ret = usbws_session_stop(wsi);
		break;
	case LWS_CALLBACK_RECEIVE:
	case LWS_CALLBACK_CLIENT_RECEIVE:
		usbws_handle_recv(wsi, in, len);
		usbws_session_handled(wsi);
		break;
	case LWS_CALLBACK_RECEIVE_PONG:
		if (!session->cont) {
			usbws_session_close_me(wsi);
			ret = -1;
			break;
		}
		usbws_session_handled(wsi);
		session->pinged = 0;
		lwsl_debug("pong %p\n", wsi);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		usbws_handle_writable(wsi);
		break;
	case USBWS_CALLBACK_HEALTH_CHECK:
		ret = usbws_check_session(wsi);
		break;
	case USBWS_CALLBACK_SEND_REQUEST:
		ret = usbws_handle_send_request(wsi);
		break;
	default:
		lwsl_debug("unhandled event %p %d\n", wsi, reason);
		break;
	}
	return ret;
}

const struct lws_protocols usbws_protocols[] = {
	{"USB/IP", usbws_handle_session,
		   sizeof(struct usbws_session), 1500, 0, NULL},
	{NULL, NULL, 0, 0, 0, NULL}
};

const char *usbws_protocol_name(void)
{
	return usbws_protocols[0].name;
}

static int usbws_send(void *arg, void *buf, int len)
{
	struct lws *wsi = (struct lws *)arg;
	struct lws_context *context = lws_get_context(wsi);
	struct usbws_session *session = wsi2session(wsi);

	lwsl_debug("send requested %p %d\n", wsi, len);
	session->send_buf = buf;
	session->send_len = len;
	session->send_offset = 0;
	usbws_request_send(context);

	usbws_cond_lock(&session->send_complete_lock);
	while (session->cont && session->send_buf)
		pthread_cond_wait(&session->send_complete_cond,
				  &session->send_complete_lock);
	usbws_cond_unlock(&session->send_complete_lock);
	lwsl_debug("send completed %p %d\n", wsi, session->send_offset);

	if (!session->cont)
		return -1;
	return session->send_offset;
}

static int usbws_recv(void *arg, void *buf, int len, int all)
{
	struct lws *wsi = (struct lws *)arg;
	struct usbws_session *session = wsi2session(wsi);
	struct list_head *p, *n;
	struct usbws_recv_buf *recv_buf;
	unsigned char *dbuf = (unsigned char *)buf;
	int rem, bytes, total = 0;

	lwsl_debug("receiving %p %p(%d)\n", arg, buf, len);

	while (session->cont) {
		usbws_cond_lock(&session->recv_queue_lock);
		while (session->cont && list_empty(&session->recv_queue))
			pthread_cond_wait(&session->recv_queue_cond,
					  &session->recv_queue_lock);
		if (!session->cont) {
			usbws_cond_unlock(&session->recv_queue_lock);
			lwsl_debug("returning read error %p\n", wsi);
			return -1;
		}
		list_for_each_safe(p, n, &session->recv_queue) {
			recv_buf = container_of(p, struct usbws_recv_buf, list);
			if (session->recv_offset >= recv_buf->len) {
				list_del(p);
				free(p);
				session->recv_offset = 0;
				continue;
			}
			rem = recv_buf->len - session->recv_offset;
			bytes = (rem > len) ? len : rem;
			memcpy(dbuf + total, recv_buf->buf, bytes);
			session->recv_offset += bytes;
			total += bytes;
			if (session->recv_offset >= recv_buf->len) {
				list_del(p);
				free(p);
				session->recv_offset = 0;
			}
			if (total >= len)
				break;
		}
		usbws_cond_unlock(&session->recv_queue_lock);
		if ((!all && total > 0) || total >= len)
			break;
	}
	lwsl_debug("received %p %d bytes\n", wsi, total);
	return total;
}

static void usbws_shutdown(void *arg)
{
	struct lws *wsi = (struct lws *)arg;
	struct usbws_session *session = wsi2session(wsi);

	lwsl_debug("shutdown session %p\n", wsi);
	usbws_session_discontinue(wsi);
}

void usbws_sock_init(struct usbip_sock *sock, struct lws *wsi)
{
	usbip_sock_init(sock, lws_get_socket_fd(wsi), wsi,
			usbws_send, usbws_recv, usbws_shutdown);
}
