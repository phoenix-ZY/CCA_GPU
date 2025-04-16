#!/usr/bin/env bash

#
# artifact evaluation:
#
# Downloads and builds all software packages
# to run a realm VM on the FVP
#
# we provide a buildroot environment to source into,
# with all host dependencies available to compile the projects.
# this script runs build scripts of all components in the repo
#

set -Eeou pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
PROJ_ROOT="$SCRIPT_DIR/.."

log_file="$SCRIPT_DIR/stdout.log"

rm "$log_file" || true

exec &> >(tee -a $log_file)

# platform
ok_init="skip"
ok_linux_cca_guest="skip"
ok_linux_cca_realm="skip"
ok_tfa="skip"
ok_kvmtool="skip"
ok_minimal_example="skip"

trap 'error_handler $? $LINENO' ERR SIGINT
trap 'error_handler $? $LINENO' SIGINT

print_status() {

  echo "Summary:"
  echo ""
  echo "platform:"
  echo "init.............................................: $ok_init"
  echo "build Aarch64 CCA-enabled NS kernel and rootfs...: $ok_linux_cca_guest"
  echo "build Aarch64 CCA-enabled Realm rootfs...........: $ok_linux_cca_realm"
  echo "build tfa, rmm...................................: $ok_tfa"
  echo "build minimal example............................: $ok_minimal_example"
  # echo "build kvmtool (VMM)..............................: $ok_kvmtool"
  echo ""
  echo "Logfile: $log_file"

}

error_handler() {
  echo "Error in install.sh script: $? occurred on $1"
  print_status
  exit 1
}

function init_repo() {
  cd $PROJ_ROOT
  ./scripts/init.sh
  ok_init="success"
}

function build_linux_cca_guest() {
  cd $PROJ_ROOT/buildconf/linux-cca-guest/
  ./setup.sh init
  ./setup.sh build
  ok_linux_cca_guest="success"
}

function build_linux_cca_realm() {
  cd $PROJ_ROOT/buildconf/linux-cca-realm/
  ./setup.sh init
  ./setup.sh build
  ok_linux_cca_realm="success"
}

function build_tfa() {
  cd $PROJ_ROOT/buildconf/tfa/
  ./setup.sh clean
  ./setup.sh init
  ./setup.sh linux
  ok_tfa="success"
}

function build_kvmtool() {
  cd $PROJ_ROOT/buildconf/shrinkwrap/
  ./setup.sh clean
  ./setup.sh init
  ./setup.sh kvmtool
  ok_kvmtool="success"
}

function build_minimal_example() {
  cd $PROJ_ROOT/src/testing/testengine
  ./build.sh
  cd $PROJ_ROOT/src/testing/testsample
  ./setup.sh
  ok_minimal_example="success"
}


function build_platform()
{
  build_linux_cca_guest
  build_linux_cca_realm
  build_tfa

  # No need to build lkvm tool all the time, just use binary asset
  # lkvmtool is built within a docker container already.
  # spawning a docker container from within distrobox causes some setup overhead.
  # So for simplicity we do not built it all the time in this run script.
  # build_kvmtool

  # build_minimal_example
}

function do_build() {
  build_platform
}

function do_init_and_build() {
  init_repo
  do_build
}

case "${1-}" in
  build)
    echo "skipping git init phase"
    time do_build
    ;;
  platform)
    time build_platform
    ;;
  *)
    time do_init_and_build
    ;;
esac

print_status
