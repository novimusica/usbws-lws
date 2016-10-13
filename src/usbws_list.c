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

	printf("\t-l, --local\n");
	printf("\t\tList local devices.\n");

#ifndef USBIP_WITH_LIBUSB
	printf("\t-uURL, --url URL\n");
	printf("\t\tURL to target USB/IP daemon.\n");

	printf("\t-xPROXY-URL, --proxy PROXY-URL\n");
	printf("\t\tProxy URL if used.\n");

	printf("\t-kKEY-FILE, --key KEY-FILE\n");
	printf("\t\tPrivate key file. Default is %s.\n", usbws_default_key);

	printf("\t-cCERT-FILE, --cert CERT-FILE\n");
	printf("\t\tCertificate file. Default is %s.\n", usbws_default_cert);

	printf("\t-VMODE, --verification MODE\n");
	printf("\t\tVerification mode - none(default) or relaxed.\n");
#endif

	printf("\t-P, --parsable\n");
	printf("\t\tParsable format.\n");

	printf("\t-h, --help\n");
	printf("\t\tPrint this help.\n");

	printf("\n");
}

static const struct option longopts[] = {
	{ "debug",        no_argument,       NULL, 'd' },
#ifdef USBIP_WITH_LIBUSB
	{ "flag",         required_argument, NULL, 'f' },
#endif
	{ "local",        no_argument,       NULL, 'l' },
#ifndef USBIP_WITH_LIBUSB
	{ "url",          required_argument, NULL, 'u' },
	{ "proxy",        required_argument, NULL, 'x' },
	{ "bus-id",       required_argument, NULL, 'b' },
	{ "key",          required_argument, NULL, 'k' },
	{ "cert",         required_argument, NULL, 'c' },
	{ "verification", required_argument, NULL, 'V' },
#endif
	{ "parsable",     no_argument,       NULL, 'p' },
	{ "help",         no_argument,       NULL, 'h' },
	{ NULL,           0,                 NULL,  0  }
};

static int opt_debug;
static int opt_local;
static const char *opt_url;
static const char *opt_proxy;
static int opt_parsable;
static int opt_help;
static struct usbws_client opt_client;
#ifdef USBIP_WITH_LIBUSB
static unsigned long opt_flags;
static const char *optstring = "df:u:x:k:c:V:lph";
#else
static const char *optstring = "du:x:k:c:V:lph";
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
		case 'l':
			opt_local = 1;
			break;
#ifndef USBIP_WITH_LIBUSB
		case 'u':
			opt_url = optarg;
			break;
		case 'x':
			opt_proxy = optarg;
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
#endif
		case 'p':
			opt_parsable = 1;
			break;
		case 'h':
		case '?':
			opt_help = 1;
			return 0;
		default:
			return -1;
		}
	}
	if (!opt_local) {
#ifndef USBIP_WITH_LIBUSB
		if (!opt_url) {
			lwsl_err("missing url and local\n");
			return -1;
		}
#else
		lwsl_err("missing local\n");
		return -1;
#endif
	}
	return 0;
}

int usbws_list(int argc, char *argv[], enum usbws_command cmd UNUSED)
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
	if (opt_local) {
		if (usbip_list_local_devices(opt_parsable))
			goto err_out;
#ifndef USBIP_WITH_LIBUSB
	} else {
		if (usbws_client_set_target(&opt_client, opt_url, opt_proxy))
			goto err_out;

		if (usbip_list_importable_devices(opt_client.host,
						  opt_client.tcp_port_s))
			goto err_out;
#endif
	}
out:
	usbws_client_free(&opt_client);
	return 0;
err_out:
	usbws_client_free(&opt_client);
	return -1;
}
