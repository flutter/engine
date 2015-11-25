#!/bin/bash
set -ex

sky/tools/gn --debug
ninja -C out/Debug generate_dart_ui
travis/analyze.sh
