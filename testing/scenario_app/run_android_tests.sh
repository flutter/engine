#!/bin/bash
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Runs the Android scenario tests on a connected device.
#   To run the test on a x64 emulator, build `android_debug_unopt_x64`, and then run
#   `./run_android_tests.sh android_debug_unopt_x64`.

set -e

# Needed because if it is set, cd may print the path it changed to.
unset CDPATH

BUILD_VARIANT=$1

# On Mac OS, readlink -f doesn't work, so follow_links traverses the path one
# link at a time, and then cds into the link destination and find out where it
# ends up.
#
# The function is enclosed in a subshell to avoid changing the working directory
# of the caller.
function follow_links() (
  cd -P "$(dirname -- "$1")"
  file="$PWD/$(basename -- "$1")"
  while [[ -L "$file" ]]; do
    cd -P "$(dirname -- "$file")"
    file="$(readlink -- "$file")"
    cd -P "$(dirname -- "$file")"
    file="$PWD/$(basename -- "$file")"
  done
  echo "$file"
)

SCRIPT_DIR=$(follow_links "$(dirname -- "${BASH_SOURCE[0]}")")
SRC_DIR="$(cd "$SCRIPT_DIR/../../.."; pwd -P)"
OUT_DIR="$SRC_DIR/out/$BUILD_VARIANT"

# function step {
#   printf "\e[1;32m+ $1\e[0m\n"
# }

# function completed {
#   printf "\e[90mDone\e[0m\n"
# }

# ADB="$SRC_DIR/third_party/android_tools/sdk/platform-tools/adb"

# function finish {
#   step "Uninstalling packages"
#   "$ADB" uninstall dev.flutter.scenarios &> /dev/null
#   "$ADB" uninstall dev.flutter.scenarios.test &> /dev/null
#   completed "Done"
# }
# trap finish EXIT

# step "Installing packages"
# "$ADB" install "$OUT_DIR"/scenario_app/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk
# "$ADB" install "$OUT_DIR"/scenario_app/app/outputs/apk/debug/app-debug.apk
# completed

# step "Running instrumented tests"
# "$ADB" shell am instrument -w dev.flutter.scenarios.test/dev.flutter.TestRunner
# completed

cd $SCRIPT_DIR

"$SRC_DIR/third_party/dart/tools/sdks/dart-sdk/bin/dart" run \
  "$SCRIPT_DIR/bin/android_integration_tests.dart" \
  --adb="$SRC_DIR"/third_party/android_tools/sdk/platform-tools/adb \
  --out-dir="$OUT_DIR"
