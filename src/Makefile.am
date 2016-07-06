AM_CFLAGS =
AM_LDFLAGS = -L@libdir@

sbin_PROGRAMS = usbws usbwsa usbwsd

usbws_SOURCES = usbws.c usbws_client.c usbws_session.c usbws_ctx.c \
		usbws_util.c usbws_connect_kind.c usbws_bind_kind.c \
		usbws_detach.c usbws_list.c usbws_port.c
usbws_CFLAGS = $(AM_CFLAGS)
usbws_LDFLAGS = -lwebsockets -pthread -lusbip -lusbipc

usbwsa_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsa_CFLAGS = $(AM_CFLAGS) -DUSBWS_APP
usbwsa_LDFLAGS = -lwebsockets -pthread -lusbip -lusbipa

usbwsd_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsd_CFLAGS = $(AM_CFLAGS) -DUSBWS_DEV
usbwsd_LDFLAGS = -lwebsockets -pthread -lusbip -lusbipd
