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
#include <usbip_api.h>
#include "usbws.h"
#include "usbws_client.h"

static void help(void)
{
	printf("\t-d, --debug\n");
	printf("\t\tPrint debugging information.\n");

#ifdef USBIP_WITH_LIBUSB
	printf("\t-fHEX, --debug-flags HEX\n");
	printf("\t\tPrint flags for stub debugging.\n");
#endif

	printf("\t-uURL, --url URL\n");
	printf("\t\tURL to target USB/IP daemon.\n");

	printf("\t-xPROXY-URL, --proxy PROXY-URL\n");
	printf("\t\tProxy URL if used.\n");

	printf("\t-bBUS-ID, --bus-id BUS-ID\n");
	printf("\t\tBus ID of a device to export.\n");

	printf("\t-iSEC, --interval SEC\n");
	printf("\t\tNo communication period to send ping-pong in second.\n");
	printf("\t\tDefault is %d. 0 denotes not to use ping pong\n",
			USBWS_PING_PONG_DEFAULT);

	printf("\t-kKEY-FILE, --key KEY-FILE\n");
	printf("\t\tPrivate key file. Default is %s.\n", usbws_default_key);

	printf("\t-cCERT-FILE, --cert CERT-FILE\n");
	printf("\t\tCertificate file. Default is %s.\n", usbws_default_cert);

	printf("\t-VMODE, --verification MODE\n");
	printf("\t\tVerification mode - none(default) or relaxed.\n");

	printf("\t-h, --help\n");
	printf("\t\tPrint this help.\n");

	printf("\n");
}

static const struct option longopts[] = {
	{ "debug",        no_argument,       NULL, 'd' },
#ifdef USBIP_WITH_LIBUSB
	{ "flag",         required_argument, NULL, 'f' },
#endif
	{ "url",          required_argument, NULL, 'u' },
	{ "proxy",        required_argument, NULL, 'x' },
	{ "bus-id",       required_argument, NULL, 'b' },
	{ "interval",     required_argument, NULL, 'i' },
	{ "key",          required_argument, NULL, 'k' },
	{ "cert",         required_argument, NULL, 'c' },
	{ "verification", required_argument, NULL, 'V' },
	{ "help",         no_argument,       NULL, 'h' },
	{ NULL,           0,                 NULL,  0  }
};

static int opt_debug;
static const char *opt_url;
static const char *opt_proxy;
static const char *opt_busid;
static int opt_help;
static struct usbws_client opt_client;
#ifdef USBIP_WITH_LIBUSB
static unsigned long opt_flags;
static const char *optstring = "df:u:x:i:b:k:c:V:h";
#else
static const char *optstring = "du:x:i:b:k:c:V:h";
#endif

static int handle_options(int argc, char *argv[])
{
	int opt;

	for (;;) {
		opt = getopt_long(argc, argv, optstring, longopts, NULL);
		if (opt == -1)
			break;

		switch (opt) {
		case 'd':
			opt_debug = 1;
			break;
#ifdef USBIP_WITH_LIBUSB
		case 'f':
			opt_flags = strtol(optarg, NULL, 16);
			break;
#endif
		case 'u':
			opt_url = optarg;
			break;
		case 'x':
			opt_proxy = optarg;
			break;
		case 'b':
			opt_busid = optarg;
			break;
		case 'i':
			usbws_ctx_set_ping_pong(client2ctx(&opt_client),
						strtol(optarg, NULL, 10));
			break;
		case 'k':
			opt_client.key = optarg;
			break;
		case 'c':
			opt_client.cert = optarg;
			break;
		case 'V':
			if (usbws_client_handle_verification(optarg,
							     &opt_client))
				return -1;
			break;
		case 'h':
		case '?':
			opt_help = 1;
			return 0;
		default:
			return -1;
		}
	}
	if (!opt_url) {
		lwsl_err("missing url\n");
		return -1;
	}
	if (!opt_busid) {
		lwsl_err("missing bus-id\n");
		return -1;
	}
	return 0;
}

int usbws_connect_kind(int argc, char *argv[], enum usbws_command cmd)
{
	int ret;

	usbws_client_init(&opt_client);

	if (handle_options(argc, argv)) {
		help();
		goto err_out;
	}
	if (opt_help) {
		help();
		goto out;
	}
	usbws_set_debug(opt_debug);
#ifdef USBIP_WITH_LIBUSB
	if (opt_flags)
		usbip_set_debug_flags(opt_flags);
#endif
	if (usbws_client_set_target(&opt_client, opt_url, opt_proxy))
		goto err_out;

	switch (cmd) {
	case USBWS_CMD_CONNECT:
		if (usbip_connect_device(opt_client.host,
					 opt_client.tcp_port_s,
					 opt_busid))
			goto err_out;
		break;
	case USBWS_CMD_DISCONNECT:
		if (usbip_disconnect_device(opt_client.host,
					 opt_client.tcp_port_s,
					 opt_busid))
			goto err_out;
		break;
#ifndef USBIP_WITH_LIBUSB
	case USBWS_CMD_ATTACH:
		if (usbip_attach_device(opt_client.host,
					 opt_client.tcp_port_s,
					 opt_busid))
			goto err_out;
		break;
#endif
	default:
		goto err_out;
	}
out:
	usbws_client_free(&opt_client);
	return 0;
err_out:
	usbws_client_free(&opt_client);
	return -1;
}
