#!/bin/sh

set -e

echo "zzzzzzzzzzzzz##########################"
xcrun --sdk iphonesimulator --show-sdk-path
echo "zzzzzzzzzzzzz##########################"
FLUTTER_ENGINE=ios_debug_sim_unopt

if [ $# -eq 1 ]; then
  FLUTTER_ENGINE=$1
fi

cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd

# Delete after LUCI push.
./compile_ios_jit.sh ../../../out/host_debug_unopt ../../../out/$FLUTTER_ENGINE/clang_x64

pushd ios/Scenarios

set -o pipefail && xcodebuild -sdk iphonesimulator \
  -scheme Scenarios \
  -destination 'platform=iOS Simulator,name=iPhone 8' \
  test \
  FLUTTER_ENGINE=$FLUTTER_ENGINE
popd
