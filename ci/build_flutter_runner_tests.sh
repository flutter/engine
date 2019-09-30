#!/bin/bash
set -ex

build_mode=${1:-debug}
build_arch=${2:-x64}
build_variant="fuchsia_${build_mode}_${build_arch}"

PATH="$HOME/depot_tools:$PATH"
cd ..

# Build the flutter runner tests far directory
flutter/tools/gn --fuchsia --no-lto --runtime-mode $build_mode --fuchsia-cpu $build_arch
ninja -C out/${build_variant} flutter/shell/platform/fuchsia/flutter:flutter_runner_tests

# Generate the far package
flutter/tools/fuchsia/gen_package.py                                \
--pm-bin $PWD/fuchsia/sdk/linux/tools/pm                            \
  --package-dir $PWD/out/${build_variant}/flutter_runner_tests_far  \
  --signing-key $PWD/flutter/tools/fuchsia/development.key          \
  --far-name flutter_runner_tests

