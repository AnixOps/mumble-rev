#!/usr/bin/env bash

set -e
set -x

source "$( dirname "$0" )/common.sh"

verify_required_env_variables_set

brew install coreutils aria2 gnu-tar xz

make_build_env_available "tar.xz"
