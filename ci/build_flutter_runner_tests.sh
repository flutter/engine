#!/bin/bash
set -ex

build_mode=${1:-debug}
build_arch=${2:-x64}
build_variant="fuchsia_${build_mode}_${build_arch}"

PATH="$HOME/depot_tools:$PATH"

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
FLUTTER_DIR=$DIR/..
SRC_DIR=$FLUTTER_DIR/..

# Build the flutter runner tests far directory
$FLUTTER_DIR/tools/gn --fuchsia --no-lto --runtime-mode $build_mode --fuchsia-cpu $build_arch
ninja -C out/${build_variant} flutter/shell/platform/fuchsia/flutter:flutter_runner_tests

# Generate the far package
$FLUTTER_DIR/tools/fuchsia/gen_package.py                               \
  --pm-bin $SRC_DIR/fuchsia/sdk/linux/tools/pm                          \
  --package-dir $SRC_DIR/out/${build_variant}/flutter_runner_tests_far  \
  --signing-key $FLUTTER_DIR/tools/fuchsia/development.key              \
  --far-name flutter_runner_tests

