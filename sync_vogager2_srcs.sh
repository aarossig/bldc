#!/usr/bin/env bash
#
# Sync source files from the voyager2 source tree into the voyager-bldc tree
# to handle commands via UART.
#
# Usage:
#   ./sync_voyager2_srcs.sh path_to_voyager2

# Fail on any error.
set -e

# Path variables.
SRC_PATH=$1
DEST_PATH=applications/app_voyager2

rm -rf $DEST_PATH
mkdir $DEST_PATH
mkdir -p $DEST_PATH/common/comms
mkdir -p $DEST_PATH/common/sys
mkdir -p $DEST_PATH/common/util
mkdir -p $DEST_PATH/external/nanopb

cp $SRC_PATH/external/nanopb/pb.h $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_common.h $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_decode.h $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_encode.h $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_common.c $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_decode.c $DEST_PATH/external/nanopb
cp $SRC_PATH/external/nanopb/pb_encode.c $DEST_PATH/external/nanopb
cp $SRC_PATH/firmware/common/comms/chibi_rfc1662_transport.h $DEST_PATH/common/comms
cp $SRC_PATH/firmware/common/comms/chibi_rfc1662_transport_impl.h $DEST_PATH/common/comms
cp $SRC_PATH/firmware/common/comms/esc_interface.pb.c $DEST_PATH/common/comms
cp $SRC_PATH/firmware/common/comms/esc_interface.pb.h $DEST_PATH/common/comms
cp $SRC_PATH/firmware/common/comms/pcm_common.h $DEST_PATH/common/comms
cp $SRC_PATH/firmware/common/sys/time.cpp $DEST_PATH/common/sys
cp $SRC_PATH/firmware/common/sys/time.h $DEST_PATH/common/sys
cp $SRC_PATH/firmware/esc/esc_host.cpp $DEST_PATH
cp $SRC_PATH/firmware/esc/esc_host.h $DEST_PATH
cp $SRC_PATH/common/comms/crc16.h $DEST_PATH/common/comms
cp $SRC_PATH/common/comms/crc16.cpp $DEST_PATH/common/comms
cp $SRC_PATH/common/comms/rfc1662_transport.h $DEST_PATH/common/comms
cp $SRC_PATH/common/comms/rfc1662_transport_impl.h $DEST_PATH/common/comms
cp $SRC_PATH/common/comms/rfc1662_transport.cpp $DEST_PATH/common/comms
cp $SRC_PATH/common/util/non_copyable.h $DEST_PATH/common/util
cp $SRC_PATH/common/util/singleton.h $DEST_PATH/common/util
cp $SRC_PATH/common/util/singleton_impl.h $DEST_PATH/common/util
