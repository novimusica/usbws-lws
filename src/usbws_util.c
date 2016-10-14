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
#include <linux/usbip_api.h>
#include <stdio.h>
#include "usbws_util.h"

int usbws_get_port(int port, int ssl)
{
	if (port)
		return port;
	else if (ssl)
		return 443;
	return 80;
}

void usbws_version(void)
{
	printf("0.0.1\n");
}

void usbws_set_debug(int opt_debug)
{
	usbip_set_use_stderr(1);

	if (opt_debug) {
		lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE |
			LLL_INFO | LLL_DEBUG, NULL);
		usbip_set_use_debug(1);
	}
}
