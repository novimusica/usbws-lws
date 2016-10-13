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
#include <getopt.h>
#include <stdio.h>
#include <linux/usbip_nppi.h>
#include "usbws_ctx.h"
#include "usbws_session.h"
#include "usbws_util.h"

#if defined(USBWS_APP)
#define USBWS_COMMAND		"usbwsa"
#define USBWS_DEFAULT_PID_FILE	"/var/run/usbwsa"
#elif defined(USBWS_DEV)
#define USBWS_COMMAND		"usbwsd"
#define USBWS_DEFAULT_PID_FILE	"/var/run/usbwsd"
#else
#define USBWS_COMMAND		"?"
#define USBWS_DEFAULT_PID_FILE	NULL
#endif

#define USBWS_DEFAULT_PATH	"usbip"
#define USBWS_DEFAULT_KEY_FILE	"cert/server.key"
#define USBWS_DEFAULT_CERT_FILE	"cert/server.crt"

static void usbws_help(void)
{
	printf("%s\n", USBWS_COMMAND);

	printf("\t-D, --daemon\n");
	printf("\t\tRun as a daemon process.\n");

	printf("\t-d, --debug\n");
	printf("\t\tPrint debugging information.\n");

#ifdef USBIP_WITH_LIBUSB
	printf("\t-fHEX, --debug-flags HEX\n");
	printf("\t\tPrint flags for stub debugging.\n");
#endif

	printf("\t-PFILE, --pid FILE\n");
	printf("\t\tWrite process id to FILE.\n");

	printf("\t-tPORT, --tcp-port PORT\n");
	printf("\t\tListen on TCP/IP port PORT.\n");

	printf("\t-pPATH, --path PATH\n");
	printf("\t\tPath to serve USB/IP. Default is %s.\n",
			USBWS_DEFAULT_PATH);

	printf("\t-iSEC, --interval SEC\n");
	printf("\t\tNo communication period to send ping-pong in second.\n");
	printf("\t\tDefault is %d. 0 denotes not to use ping pong\n",
			USBWS_PING_PONG_DEFAULT);

	printf("\t-s, --ssl\n");
	printf("\t\tEnable SSL.\n");

	printf("\t-kKEY-FILE, --key KEY-FILE\n");
	printf("\t\tPrivate key file. Default is %s.\n",
			USBWS_DEFAULT_KEY_FILE);

	printf("\t-cCERT-FILE, --cert CERT-FILE\n");
	printf("\t\tCertificate file. Default is %s.\n",
			USBWS_DEFAULT_CERT_FILE);

	printf("\t-h, --help\n");
	printf("\t\tPrint this help.\n");

	printf("\t-v, --version\n");
	printf("\t\tShow version.\n");

	printf("\n");
}

static int opt_daemonize;
static int opt_debug;
#ifdef USBIP_WITH_LIBUSB
static unsigned long opt_flags;
#endif
static int opt_tcp_port;
static const char *opt_path = USBWS_DEFAULT_PATH;
static int opt_ssl;
static const char *opt_pid_file;
static const char *opt_key_file = USBWS_DEFAULT_KEY_FILE;
static const char *opt_cert_file = USBWS_DEFAULT_CERT_FILE;
static int opt_help;
static int opt_version;

static struct usbws_ctx service_ctx;

static int usbws_handle_options(int argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "daemon",       no_argument,       NULL, 'D' },
		{ "debug",        no_argument,       NULL, 'd' },
#ifdef USBIP_WITH_LIBUSB
		{ "flag",         required_argument, NULL, 'f' },
#endif
		{ "pid",          optional_argument, NULL, 'P' },
		{ "tcp-port",     required_argument, NULL, 't' },
		{ "path",         required_argument, NULL, 'p' },
		{ "interval",     required_argument, NULL, 'i' },
		{ "ssl",          no_argument,       NULL, 's' },
		{ "key",          required_argument, NULL, 'k' },
		{ "cert",         required_argument, NULL, 'c' },
		{ "help",         no_argument,       NULL, 'h' },
		{ "version",      no_argument,       NULL, 'v' },
		{ NULL,           0,                 NULL,  0  }
	};
	int opt;

	for (;;) {
		opt = getopt_long(argc, argv, "Ddf:P::t:p:i:sk:c:hv",
				  longopts, NULL);
		if (opt == -1)
			break;
		switch (opt) {
		case 'D':
			opt_daemonize = 1;
			break;
		case 'd':
			opt_debug = 1;
			break;
#ifdef USBIP_WITH_LIBUSB
		case 'f':
			opt_flags = strtol(optarg, NULL, 16);
			break;
#endif
		case 'P':
			opt_pid_file = optarg ? optarg : USBWS_DEFAULT_PID_FILE;
			break;
		case 't':
			opt_tcp_port = strtol(optarg, NULL, 10);
			break;
		case 'p':
			opt_path = optarg;
			break;
		case 'i':
			usbws_ctx_set_ping_pong(&service_ctx,
						strtol(optarg, NULL, 10));
			break;
		case 's':
			opt_ssl = 1;
			break;
		case 'k':
			opt_key_file = optarg;
			break;
		case 'c':
			opt_cert_file = optarg;
			break;
		case 'v':
			opt_version = 1;
			return 0;
		case 'h':
		case '?':
			opt_help = 1;
			return 0;
		default:
			return -1;
		}
	}
	return 0;
}

static void *usbws_service_session(void *arg)
{
	struct lws *wsi = (struct lws *)arg;
	struct usbws_session *session = wsi2session(wsi);
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	struct sockaddr_in adr;
	socklen_t adrlen = sizeof(adr);
	int sockfd = lws_get_socket_fd(wsi);
	struct usbip_sock sock;

	if (getpeername(sockfd, (struct sockaddr *)&adr, &adrlen)) {
		lwsl_err("failed to get peer name\n");
		return NULL;
	}
	if (getnameinfo((struct sockaddr *)&adr, adrlen,
			host, NI_MAXHOST, port, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV)) {
		lwsl_err("failed to get name info\n");
		return NULL;
	}
	usbws_sock_init(&sock, wsi);

	lwsl_debug("servicing session %p %s:%s\n", wsi, host, port);
	if (usbip_recv_pdu(&sock, host, port))
		lwsl_err("failed to recv pdu\n");
	lwsl_debug("end of service session %p %s:%s\n", wsi, host, port);

	return NULL;
}

static int usbws_service_start_session(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);

	lwsl_info("starting service session %p\n", wsi);
	if (pthread_create(&session->tid, NULL, usbws_service_session, wsi)) {
		lwsl_err("failed to create service session\n");
		return -1;
	}
	return 0;
}

static int usbws_service_stop_session(struct lws *wsi)
{
	struct usbws_session *session = wsi2session(wsi);

	lwsl_info("waiting service session %p\n", wsi);
	if (session->tid)
		pthread_join(session->tid, NULL);
	lwsl_info("end of service session %p\n", wsi);
	return 0;
}

static int usbws_service(void)
{
	struct lws_context_creation_info info;
	struct lws_context *context;
	int timeout = usbws_ctx_get_ping_pong(&service_ctx) * 1000;
	int sent_ping = 0;

	usbws_set_info(&info, &service_ctx,
		       usbws_get_port(opt_tcp_port, opt_ssl),
		       opt_ssl,
		       opt_key_file,
		       opt_cert_file);
	/*
	 * TODO:
	 * Handle with multiple context
	 * which can be created by info.count_threads
	 */

	context = usbws_ctx_create(&service_ctx, &info);
	if (!context) {
		lwsl_err("failed to create context\n");
		return -1;
	}

	usbws_set_sigint(&service_ctx);

	lwsl_info("started service\n");
	while (!usbws_ctx_stopped(&service_ctx)) {
		if (lws_service(context, timeout))
			break;
		usbws_health_check(context);
	}
	lwsl_info("end of service\n");

	usbws_ctx_destroy(&service_ctx);

	return 0;
}

static int usbws_create_pid_file(void)
{
	FILE *fp;

	fp = fopen(opt_pid_file, "w");
	if (!fp) {
		lwsl_err("failed to create pid file\n");
		goto err_out;
	}
	if (fprintf(fp, "%d\n", getpid()) < 1) {
		lwsl_err("failed to create pid file\n");
		goto err_close;
	}
	fclose(fp);
	return 0;
err_close:
	fclose(fp);
err_out:
	return -1;
}

static void usbws_remove_pid_file(void)
{
	unlink(opt_pid_file);
}

int main(int argc, char *argv[])
{
	int i;

	usbws_ctx_init(&service_ctx,
		       usbws_service_start_session,
		       usbws_service_stop_session);

	if (usbws_handle_options(argc, argv)) {
		usbws_help();
		return 0;
	}
	if (opt_help) {
		usbws_help();
		return 0;
	}
	if (opt_version) {
		usbws_version();
		return 0;
	}
	usbws_set_debug(opt_debug);
#ifdef USBIP_WITH_LIBUSB
	if (opt_flags)
		usbip_set_debug_flags(opt_flags);
#endif
	if (opt_pid_file) {
		if (usbws_create_pid_file())
			goto err_out;
	}
	if (usbip_open_driver()) {
		lwsl_err("failed to open driver\n");
		goto err_rm_pid_file;
	}
#ifdef __unix__
	if (opt_daemonize) {
		if (daemon(0, 0) < 0) {
			lwsl_err("failed to daemonizing\n");
			goto err_close_driver;
		}
		umask(0);
		usbip_set_use_syslog(1);
	}
#endif

	usbws_service();

	usbip_close_driver();

	if (opt_pid_file)
		usbws_remove_pid_file();

	return 0;
err_close_driver:
	usbip_close_driver();
err_rm_pid_file:
	if (opt_pid_file)
		usbws_remove_pid_file();
err_out:
	return -1;
}
