AM_LDFLAGS = -L@libdir@ -lwebsockets -pthread

if !WITH_LIBUSB
AM_CFLAGS =
LDFLAGS_CMD = -lusbip -lusbipc
LDFLAGS_DAEMON_DEV = -lusbip -lusbipd
LDFLAGS_DAEMON_APP = -lusbip -lusbipa

sbin_PROGRAMS = usbws usbwsd usbwsa

usbws_SOURCES = usbws.c usbws_client.c usbws_session.c usbws_ctx.c \
		usbws_util.c usbws_connect_kind.c usbws_bind_kind.c \
		usbws_list.c usbws_detach.c usbws_port.c
else
AM_CFLAGS = -DUSBIP_WITH_LIBUSB
LDFLAGS_CMD = -lusbip_libusb -lusbip_stub -lusbipc_libusb
LDFLAGS_DAEMON_DEV = -lusbip_libusb -lusbip_stub -lusbipd_libusb
LDFLAGS_DAEMON_APP =

sbin_PROGRAMS = usbws usbwsd

usbws_SOURCES = usbws.c usbws_client.c usbws_session.c usbws_ctx.c \
		usbws_util.c usbws_connect_kind.c usbws_bind_kind.c \
		usbws_list.c
endif

usbws_CFLAGS = $(AM_CFLAGS)
usbws_LDFLAGS = $(AM_LDFLAGS) $(LDFLAGS_CMD)

usbwsd_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsd_CFLAGS = $(AM_CFLAGS) -DUSBWS_DEV
usbwsd_LDFLAGS = $(AM_LDFLAGS) $(LDFLAGS_DAEMON_DEV)

if !WITH_LIBUSB
usbwsa_SOURCES = usbwsd.c usbws_session.c usbws_ctx.c usbws_util.c
usbwsa_CFLAGS = $(AM_CFLAGS) -DUSBWS_APP
usbwsa_LDFLAGS = $(AM_LDFLAGS) $(LDFLAGS_DAEMON_APP)
endif
