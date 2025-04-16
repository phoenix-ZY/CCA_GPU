#!/usr/bin/env bash
set -euo pipefail

readonly SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
readonly FVP_URL=https://developer.arm.com/-/media/Files/downloads/ecosystem-models/FVP_Base_RevC-2xAEMvA_11.20_15_Linux64.tgz

set -x

mkdir -p $SCRIPT_DIR/build
cd $SCRIPT_DIR/build

# fvp
readonly FVP_TGZ=FVP_Base_RevC-2xAEMvA_11.20_15_Linux64.tgz
readonly FVP_OUT_DIR=$SCRIPT_DIR/build/FVP

readonly FVP_BIN_DIR=$FVP_OUT_DIR/Base_RevC_AEMvA_pkg/models/Linux64_GCC-9.3/
readonly FVP_BIN=$FVP_BIN_DIR/FVP_Base_RevC-2xAEMvA

if [ ! -f "$FVP_TGZ" ]; then
    wget ${FVP_URL} -O $FVP_TGZ
    mkdir -p $FVP_OUT_DIR
    tar -xf $FVP_TGZ -C $FVP_OUT_DIR
fi
mkdir -p $SCRIPT_DIR/bin

ln --relative -s $FVP_BIN $SCRIPT_DIR/bin/FVP_Base_RevC-2xAEMvA && true # ignore existing link
export PATH="$PATH:$FVP_BIN_DIR"
