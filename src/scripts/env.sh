#!/usr/bin/env bash
export ROOT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# ext
export EXT_DIR=$ROOT_DIR/ext
export EXT_BUILDROOT_DIR=$EXT_DIR/buildroot
export EXT_PTEDIT_DIR=$ROOT_DIR/ext/pteditor

# src
export SRC_DIR=$ROOT_DIR/src
export RAMDISK_DIR=/tmp/armcca-ramdisk



# buildroot output
#
# in distrobox container, set OUTPUT_DISTROBOX="-distrobox"
# to have separate output artifact when built in container
#
out_suffix=${OUTPUT_SUFFIX:=}
# export OUTPUT_DIR=$ROOT_DIR/output${out_suffix}
export OUTPUT_DIR=$ROOT_DIR/output
export OUTPUT_LINUX_GUEST_DIR=$OUTPUT_DIR/buildroot-linux-guest
export OUTPUT_LINUX_GUEST_HEADERS=$OUTPUT_LINUX_GUEST_DIR/build/linux-custom

export OUTPUT_LINUX_CCA_GUEST_DIR=$OUTPUT_DIR/buildroot-linux-cca-guest
export OUTPUT_LINUX_CCA_GUEST_HEADERS=$OUTPUT_LINUX_CCA_GUEST_DIR/build/linux-custom

export OUTPUT_LINUX_CCA_REALM_DIR=$OUTPUT_DIR/buildroot-linux-cca-realm
export OUTPUT_LINUX_CCA_REALM_HEADERS=$OUTPUT_LINUX_CCA_REALM_DIR/build/linux-custom

export OUTPUT_LINUX_HOST_DIR=$OUTPUT_DIR/buildroot-linux-host
export OUTPUT_LINUX_HOST_HEADERS=$OUTPUT_LINUX_HOST_DIR/build/linux-custom

# buildconf
export BUILDCONF_DIR=$ROOT_DIR/buildconf

# assets
export ASSETS_DIR=$ROOT_DIR/assets

# scripts
export SCRIPTS_DIR=$ROOT_DIR/scripts

export PATH="$PATH:$ASSETS_DIR/fvp/bin"
export PATH="$PATH:$SCRIPTS_DIR"
