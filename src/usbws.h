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

#ifndef __USBWS_H
#define __USBWS_H

enum usbws_command {
	USBWS_CMD_CONNECT,
	USBWS_CMD_DISCONNECT,
	USBWS_CMD_ATTACH,
	USBWS_CMD_DETACH,
	USBWS_CMD_PORT,
	USBWS_CMD_LIST,
	USBWS_CMD_BIND,
	USBWS_CMD_UNBIND,
	USBWS_CMD_HELP,
	USBWS_CMD_VERSION,
	USBWS_CMD_ERROR,
};

int usbws_connect_kind(int argc, char *argv[], enum usbws_command cmd);
int usbws_list(int argc, char *argv[], enum usbws_command cmd);
int usbws_port(int argc, char *argv[], enum usbws_command cmd);
int usbws_bind_kind(int argc, char *argv[], enum usbws_command cmd);
int usbws_detach(int argc, char *argv[], enum usbws_command cmd);

#endif /* !__USBWS_H */
