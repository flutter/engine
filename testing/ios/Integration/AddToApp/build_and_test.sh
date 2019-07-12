#!/bin/bash

set -e

cd "$(dirname "$0")"

PRETTY="tee"
if which xcpretty; then
  PRETTY="xcpretty"
fi

# The Flutter path comes from where ever CI would clone it into.
# This comes from LUCI and ideally from Cirrus pre-submit one day.
FLUTTER="$FRAMEWORK_PATH"
# If flutter is already in path (such as when testing locally), just use
# that instead.
if which flutter; then
  FLUTTER="flutter"
fi

ENGINE_PATH="$(realpath ../../../../..)"
if [ ! -d "$ENGINE_PATH/out/ios_debug_sim_unopt" ] || [ ! -d "$ENGINE_PATH/out/host_debug_sim_unopt" ]; then
  echo "Failed to locate an iOS engine at ios_debug_sim_unopt and host_debug_sim_unopt at $ENGINE_PATH/out."
  echo "If running locally, did you build the engine first?"
  exit 1
fi

pushd FlutterModule

echo "Building module at $PWD"

$FLUTTER build ios --debug --no-codesign --simulator \
  --local-engine=ios_debug_sim_unopt --local-engine-src-path $ENGINE_PATH

pushd .ios
# xcodebuild -workspace Runner.xcworkspace -scheme Runner -destination generic/platform=iOS -configuration Debug CODE_SIGNING_REQUIRED=NO CODE_SIGNING_IDENTITY="" CODE_SIGNING_ALLOWED=NO
# xcodebuild -workspace Runner.xcworkspace -scheme Runner -destination generic/platform=iOS -configuration Profile CODE_SIGNING_REQUIRED=NO CODE_SIGNING_IDENTITY="" CODE_SIGNING_ALLOWED=NO
# xcodebuild -workspace Runner.xcworkspace -scheme Runner -destination generic/platform=iOS -configuration Release CODE_SIGNING_REQUIRED=NO CODE_SIGNING_IDENTITY="" CODE_SIGNING_ALLOWED=NO
popd
popd

pod install
os_version=$(xcrun --show-sdk-version --sdk iphonesimulator)
set -o pipefail && xcodebuild -workspace OuterApp.xcworkspace \
  -scheme OuterAppTests -sdk "iphonesimulator$os_version" \
  -destination "OS=$os_version,name=iPhone X" test | $PRETTY
