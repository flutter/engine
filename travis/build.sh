#!/bin/bash
set -ex

# Linux Debug
./sky/tools/gn --release
ninja -j 4 -C out/Release
./sky/tools/test_sky --release --child-processes=1
