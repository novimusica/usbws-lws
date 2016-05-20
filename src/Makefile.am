AM_CPPFLAGS = -I$(top_srcdir)/../libsrc -I$(top_srcdir)/../src

bin_PROGRAMS = usbws usbwsa usbwsd

usbws_LDFLAGS = -lwebsockets -pthread
usbws_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbip.la
usbws_SOURCES = usbws.c usbws_client.c usbws_session.c usbws_ctx.c \
		usbws_util.c usbws_connect_kind.c usbws_bind_kind.c \
		usbws_detach.c usbws_list.c usbws_port.c
usbws_CPPFLAGS = $(AM_CPPFLAGS)

usbwsa_LDFLAGS = -lwebsockets -pthread
usbwsa_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbipa.la
usbwsa_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsa_CPPFLAGS = $(AM_CPPFLAGS) -DUSBWS_APP

usbwsd_LDFLAGS = -lwebsockets -pthread
usbwsd_LDADD = $(top_builddir)/../libsrc/libusbiplib.la \
		$(top_builddir)/../src/libusbipd.la
usbwsd_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsd_CPPFLAGS = $(AM_CPPFLAGS) -DUSBWS_DEV
