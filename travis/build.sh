#!/bin/bash
set -ex

# Linux Debug
./sky/tools/gn --debug --no-clang
ninja -j 8 -C out/Debug
./sky/tools/test_sky --debug
