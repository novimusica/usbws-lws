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
#include "usbws_util.h"

static void help(void)
{
	printf("\t-d, --debug\n");
	printf("\t\tPrint debugging information.\n");

#ifdef USBIP_WITH_LIBUSB
	printf("\t-fHEX, --debug-flags HEX\n");
	printf("\t\tPrint flags for stub debugging.\n");
#endif

	printf("\t-bBUS-ID, --bus-id BUS-ID\n");
	printf("\t\tBus ID of a device to export.\n");

	printf("\t-h, --help\n");
	printf("\t\tPrint this help.\n");

	printf("\n");
}

static const struct option longopts[] = {
	{ "debug",        no_argument,       NULL, 'd' },
#ifdef USBIP_WITH_LIBUSB
	{ "flag",         required_argument, NULL, 'f' },
#endif
	{ "bus-id",       required_argument, NULL, 'b' },
	{ "help",         no_argument,       NULL, 'h' },
	{ NULL,           0,                 NULL,  0  }
};

static int opt_debug;
static const char *opt_busid;
static int opt_help;
#ifdef USBIP_WITH_LIBUSB
static unsigned long opt_flags;
static const char *optstring = "df:b:h";
#else
static const char *optstring = "db:h";
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
		case 'b':
			opt_busid = optarg;
			break;
		case 'h':
		case '?':
			opt_help = 1;
			return 0;
		default:
			return -1;
		}
	}
	if (!opt_busid) {
		lwsl_err("missing bus-id\n");
		return -1;
	}
	return 0;
}

int usbws_bind_kind(int argc, char *argv[], enum usbws_command cmd)
{
	if (handle_options(argc, argv)) {
		help();
		return -1;
	}
	if (opt_help) {
		help();
		return 0;
	}
	usbws_set_debug(opt_debug);
#ifdef USBIP_WITH_LIBUSB
	if (opt_flags)
		usbip_set_debug_flags(opt_flags);
#endif

	switch (cmd) {
	case USBWS_CMD_BIND:
		return usbip_bind_device(opt_busid);
	case USBWS_CMD_UNBIND:
		return usbip_unbind_device(opt_busid);
	}
	return -1;
}
