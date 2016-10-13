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
#include "usbws_util.h"

#if defined(USBWS_APP)
#define USBWS_DEFAULT_PID_FILE	"/var/run/usbwsa"
#elif defined(USBWS_DEV)
#define USBWS_DEFAULT_PID_FILE	"/var/run/usbwsd"
#else
#define USBWS_DEFAULT_PID_FILE	NULL
#endif

#define USBWS_DEFAULT_PATH	"usbip"
#define USBWS_DEFAULT_KEY_FILE	"cert/server.key"
#define USBWS_DEFAULT_CERT_FILE	"cert/server.crt"

static void __help(void)
{
#ifdef USBIP_WITH_LIBUSB
	printf("usbws <connect | disconnect | list | bind | unbind\n");
	printf("       | help | version>\n");
#else
	printf("usbws <connect | disconnect | attach | detach\n");
	printf("       | list | bind | unbind | help | version>\n");
#endif
	printf("      [ options ]\t");
}

static inline int help(int argc UNUSED, char *argv[] UNUSED,
		enum usbws_command cmd UNUSED)
{
	__help();
	return 0;
}

static int version(int argc UNUSED, char *argv[] UNUSED,
		   enum usbws_command cmd UNUSED)
{
	usbws_version();
	return 0;
}

static struct usbws_cmd_list {
	enum usbws_command id;
	const char *name;
	int (*proc)(int argc, char *argv[], enum usbws_command cmd);
} usbws_commands[] = {
	{USBWS_CMD_CONNECT, "connect", usbws_connect_kind},
	{USBWS_CMD_DISCONNECT, "disconnect", usbws_connect_kind},
#ifndef USBIP_WITH_LIBUSB
	{USBWS_CMD_ATTACH, "attach", usbws_connect_kind},
	{USBWS_CMD_DETACH, "detach", usbws_detach},
	{USBWS_CMD_PORT, "port", usbws_port},
#endif
	{USBWS_CMD_LIST, "list", usbws_list},
	{USBWS_CMD_BIND, "bind", usbws_bind_kind},
	{USBWS_CMD_UNBIND, "unbind", usbws_bind_kind},
	{USBWS_CMD_HELP, "help", help},
	{USBWS_CMD_VERSION, "version", version},
	{USBWS_CMD_ERROR, NULL}
};

int main(int argc, char *argv[])
{
	int i, ret = -1;

	if (argc < 2)
		return 1;
	for (i = 0; usbws_commands[i].name; i++) {
		if (strcmp(argv[1], usbws_commands[i].name) == 0) {
			ret = usbws_commands[i].proc(argc, argv,
						     usbws_commands[i].id);
			break;
		}
	}
	if (!usbws_commands[i].name) {
		__help();
		return 1;
	}
	return ret ? 1 : 0;
}
