AM_CFLAGS = -I$(top_srcdir)/../libsrc -I$(top_srcdir)/../src

bin_PROGRAMS = usbws usbwsa usbwsd

usbws_SOURCES = usbws.c usbws_client.c usbws_session.c usbws_ctx.c \
		usbws_util.c usbws_connect_kind.c usbws_bind_kind.c \
		usbws_detach.c usbws_list.c usbws_port.c
usbws_CFLAGS = $(AM_CFLAGS)
usbws_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbip.la
usbws_LDFLAGS = -lwebsockets -pthread

usbwsa_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsa_CFLAGS = $(AM_CFLAGS) -DUSBWS_APP
usbwsa_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbipa.la
usbwsa_LDFLAGS = -lwebsockets -pthread

usbwsd_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsd_CFLAGS = $(AM_CFLAGS) -DUSBWS_DEV
usbwsd_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbipd.la
usbwsd_LDFLAGS = -lwebsockets -pthread
