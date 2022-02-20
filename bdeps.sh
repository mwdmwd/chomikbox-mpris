#!/bin/sh
set -e

[ "$(id -u)" -ne 0 ] && exec sudo "$0"

dpkg --add-architecture i386
apt-get update -y
apt-get install -y --no-install-recommends build-essential gcc-multilib g++-mingw-w64-i686 wine wine32-tools libglib2.0-dev:i386
