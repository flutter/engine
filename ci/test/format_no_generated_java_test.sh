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
SRC_DIR="$(
  cd "$SCRIPT_DIR/../../.."
  pwd -P
)"
FLUTTER_DIR="$SRC_DIR/flutter"

# Creates a file named `GeneratedPluginRegistrant.java` in the project.
# This file is generated by Flutter tooling and should not be checked in.
touch "$FLUTTER_DIR/GeneratedPluginRegistrant.java"

# Create a trap that, on exit, removes the file.
function cleanup() {
  rm -f "$FLUTTER_DIR/GeneratedPluginRegistrant.java"
}
trap cleanup EXIT

# Runs ../format.sh and verifies that it fails with the expected error message.
# If it fails, the script prints an error message and exits with a non-zero status.
"$FLUTTER_DIR/ci/format.sh" || {
  echo "PASS: analyze.sh failed as expected"
  exit 0
}

echo "FAIL: format.sh did not fail as expected"
exit 1
