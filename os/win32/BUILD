VisualStudio solution structure example

[For usbws and usbwsd]

$(SolutionDir)
|
+-getopt
|   Type:    dll
|   Sources: getopt.c
|   Headers: getopt.h
|   NOTE:    these files are excluded. Use libwebsockets/win32port/win32helpers,
|            or other compatible open source.
|
+-lib
|   Type:    lib
|   Sources: usbip_network.c usbip_common.c usbip_host_nppi.c names.c
|            stub_*.c # from libusb/stub
|            dummy_device_driver.c # from libusb/libsrc
|   Headers: usbip_network.h usbip_common.h usbip_config.h
|            usbip_host_common.h usbip_host_driver.h usbip_device_driver.h
|            names.h list.h
|            stub_common.h stub.h # from libusb/stub
|            usbip_os.h usbip_sock.h # from libusb/os
|   Includes: $(SolutionDir);$(SolutionDir)\win32
|   Definitions: USBIP_WITH_LIBUSB;HAVE_CONFIG_H;USBIP_AS_LIBRARY
|
+-libusb-1.0 (dll)
|   Added as an existing project from <libusb-src>\msvc\libusb_dll_<ver>.vcxproj
|   OutputDir: $(SolutionDir)$(Configuration)\
|
+-usbip
|   Type:    lib
|   Sources: usbip_connect.c usbip_disconnect.c usbip_list.c
|   Headers: usbip.h
|   Includes: $(SolutionDir);$(SolutionDir)\lib;$(SolutionDir)\win32
|   Definitions: USBIP_WITH_LIBUSB;HAVE_CONFIG_H;USBIP_AS_LIBRARY;
|            USBIDS_FILE="..\\usb.ids"
|
+-usbipd
|   Type:    lib
|   Sources: usbipd_dev.c
|   Headers: usbipd.h
|   Includes: $(SolutionDir);$(SolutionDir)\lib;$(SolutionDir)\win32
|   Definitions: USBIP_WITH_LIBUSB;HAVE_CONFIG_H;USBIP_AS_LIBRARY
|
+-win32
|   Headers: usbip_os.h usbip_sock.h # from libusb/os/win32
|
+linux
|   Headers: usbip_nppi.h # from include/uapi/linux
|
+-config.h
|   Copied from libusb/os/win32
|
+-usb.ids
|   Copied from /usr/share/hwdata/ or downloaded from net.
|   AccentAcute character must be modified to avoid invalid ascii exception.
|   ex) sed -i -e 's/\xb4/\x00/' usb.ids
|
+-websockets_shared
|   Added as an existing project from <libwebsockets-src>\build\
|   websockets_shared.vcxproj.
|   Depending Files: ws2_32.lib
|		<OpenSSL-Win32>\lib\VC\static\libeay32MD.lib
|		<OpenSSL-Win32>\lib\VC\static\libeay32MT.lib
|		<OpenSSL-Win32>\lib\VC\static\ssleay32MD.lib
|		<OpenSSL-Win32>\lib\VC\static\ssleay32MT.lib
|   OutputDir: $(SolutionDir)$(Configuration)\
|
+-ZERO_CHECK
|   Added as an existing project from <libwebsockets-src>\build\
|   ZERO_CHECK.vcxproj.
|   Referenced from websockets_shared.
|   OutputDir: $(SolutionDir)$(Configuration)\
|
+-zlib_internal
|   Added as an existing project from <libwebsockets-src>\build\
|   Referenced from websockets_shared.
|   zlib_internal.vcxproj
|   OutputDir: $(SolutionDir)$(Configuration)\
|
+-usbws
|   Type:    exe
|   Sources: usbws.c usbws_ctx.c usbws_session.c usbws_util.c
|            usbws_client.c
|            usbws_connect_kind.c usbws_bind_kind.c usbws_list.c
|   Headers: usbws.h usbws_ctx.h usbws_session.h usbws_util.h wsbws_win32.h
|            usbws_client.h
|   Includes: $(SolutionDir)\getopt
|            $(SolutionDir)\lib
|            <libwebsockets-src>\lib
|            <libwebsockets-src>\build
|            <open-ssl-src>\include
|   Definitions: USBIP_WITH_LIBUSB
|   Depending Files: ws2_32.lib
|   Use Library Dependency Inputs: yes
|   Reference: getopt;lib;libusb-1.0 (dll);usbip;win32;websockets_shared
|   NOTE:    libwebsockets.h must be incuded first to avoid conflicts.
|
+-usbwsd
|   Type:    exe
|   Sources: usbwsd.c usbws_ctx.c usbws_session.c usbws_util.c
|   Headers: usbws_ctx.h usbws_session.h usbws_util.h wsbws_win32.h
|   Includes: $(SolutionDir)\getopt
|            $(SolutionDir)\lib
|            <libwebsockets-src>\lib
|            <libwebsockets-src>\build
|            <open-ssl-src>\include
|   Definitions: USBIP_WITH_LIBUSB
|   Use Library Dependency Inputs: yes
|   Depending Files: ws2_32.lib
|   Reference: getopt;lib;libusb-1.0 (dll);usbipd;win32;websockets_shared
|   NOTE:    libwebsockets.h must be incuded first to avoid conflicts.
|
+-libusb-1.0
|   SYMLINK to <libusb-src>\libusb
|   ex) mklink /D libusb-1.0 <libusb-src>\libusb
+-Debug/Release
    copy <poco-src>\bin\* .

NOTE:
    To support Windows 7 and Visual Studio 2012, programs must not be c99.

    Winsock headers must be included at first than any other headers.
    Please, check the position which includes usbip_sock.h if duplicated
    definition errors relating  winsock headers occur.
