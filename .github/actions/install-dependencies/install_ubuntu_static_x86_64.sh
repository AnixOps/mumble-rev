#!/usr/bin/env bash

set -e
set -x

source "$( dirname "$0" )/common.sh"

sudo apt update

sudo apt -y install \
	build-essential \
	g++-multilib \
	ninja-build \
	`# Still required for qtbase vcpkg package` \
	'^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libegl1-mesa-dev \
	`# TODO: can we get rid of these by replacing with vcpkg packages?` \
	libsm-dev \
	libspeechd-dev \
	libavahi-compat-libdnssd-dev \
	libasound2-dev

verify_required_env_variables_set

make_build_env_available "tar.xz"
