#!/bin/bash
#
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

# Needed because if it is set, cd may print the path it changed to.
unset CDPATH

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
SRC_DIR="$(cd "$SCRIPT_DIR/../.."; pwd -P)"
FLUTTER_DIR="$SRC_DIR/flutter"
DART_BIN="$SRC_DIR/out/host_debug_unopt/dart-sdk/bin"
DART="$DART_BIN/dart"

if [[ ! -f "$DART" ]]; then
  echo "'$DART' not found"
  echo ""
  echo "To build the Dart SDK, run:"
  echo "  flutter/tools/gn --unoptimized --runtime-mode=debug"
  echo "  ninja -C out/host_debug_unopt"
  exit 1
fi

# Invoke the implementation half of this analyze script.
./analyze_impl.sh --dart="$DART" --flutter-dir="$FLUTTER_DIR"
