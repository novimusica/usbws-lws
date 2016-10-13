#!/bin/sh

DIR=`dirname $0`
CMD=`basename $0`
LINUX=$1

TOP=$LINUX/tools/usb/usbip
INC=$LINUX/include
WS_SRC=$DIR/../../src
HWDATA=/usr/share/hwdata/

if [ -z "$1" -o -z "$2" ]; then
	echo "Usage: $CMD <linux-drc-dir> <distination-dir>"
	exit 1
fi

DST_SOL=$2

DST_LIB=$DST_SOL/lib
DST_USBIP=$DST_SOL/usbip
DST_USBIPD=$DST_SOL/usbipd
DST_WIN=$DST_SOL/win32
DST_LINUX=$DST_SOL/linux
DST_CFG=$DST_SOL
DST_IDS=$DST_SOL/usb.ids
DST_USBWS=$DST_SOL/usbws
DST_USBWSD=$DST_SOL/usbwsd

FILES_LIB="\
	$TOP/src/usbip_network.[ch] \
	$TOP/libsrc/usbip_common.[ch] \
	$TOP/libsrc/usbip_host_nppi.c \
	$TOP/libsrc/usbip_config.h \
	$TOP/libsrc/usbip_host_common.h \
	$TOP/libsrc/usbip_host_driver.h \
	$TOP/libsrc/usbip_device_driver.h \
	$TOP/libsrc/names.[ch] \
	$TOP/libsrc/list.h \
	$TOP/libusb/stub/*.[ch] \
	$TOP/libusb/libsrc/dummy_device_driver.c \
	$TOP/libusb/os/usbip_os.h \
	$TOP/libusb/os/usbip_sock.h"
FILES_USBIP="\
	$TOP/src/usbip.h \
	$TOP/src/usbip_connect.c \
	$TOP/src/usbip_disconnect.c \
	$TOP/src/usbip_list.c"
FILES_USBIPD="\
	$TOP/src/usbipd.h \
	$TOP/src/usbipd_dev.c"
FILES_WIN="\
	$TOP/libusb/os/win32/usbip_os.h \
	$TOP/libusb/os/win32/usbip_sock.h"
FILES_LINUX="\
	$INC/uapi/linux/usbip_nppi.h"
FILE_CFG=$TOP/libusb/os/win32/config.h
FILE_IDS=$HWDATA/usb.ids
FILES_USBWS="\
	$WS_SRC/usbws.[ch] \
	$WS_SRC/usbws_client.[ch] \
	$WS_SRC/usbws_session.[ch] \
	$WS_SRC/usbws_ctx.[ch] \
	$WS_SRC/usbws_util.[ch] \
	$WS_SRC/usbws_connect_kind.c \
	$WS_SRC/usbws_bind_kind.c \
	$WS_SRC/usbws_list.c \
	$WS_SRC/usbws_win32.h"
FILES_USBWSD="\
	$WS_SRC/usbwsd.[ch] \
	$WS_SRC/usbws_session.[ch] \
	$WS_SRC/usbws_util.[ch] \
	$WS_SRC/usbws_ctx.[ch] \
	$WS_SRC/usbws_win32.h"

cp $FILES_LIB $DST_LIB
cp $FILES_USBIP $DST_USBIP
cp $FILES_USBIPD $DST_USBIPD
cp $FILES_WIN $DST_WIN
cp $FILES_LINUX $DST_LINUX
cp $FILE_CFG $DST_CFG
cp $FILES_USBWS $DST_USBWS
cp $FILES_USBWSD $DST_USBWSD

sed -e 's/\xb4/\x20/' $FILE_IDS > $DST_IDS
echo "AccentAcute character(s) in $DST_IDS has been modified."
