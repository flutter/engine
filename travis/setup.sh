#!/bin/bash
set -ex

(cd; git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git)

PATH="$HOME/depot_tools:$PATH"

cd ..
mv engine flutter
mkdir src
mv flutter src
cd src

mv flutter/travis/gclient ../.gclient
gclient sync

../tools/dart/update.py

PATH="$PWD/../dart/tools/sdks/linux/dart-sdk/bin:$PATH"
