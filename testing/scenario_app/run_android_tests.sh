#!/bin/sh
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Runs the Android scenario tests on a connected device.

if [ -z $ENGINE_PATH ]
then
  echo "Please set ENGINE_PATH environment variable. e.g. \`export ENGINE_PATH='<engine-path>'\`"
  exit 1
fi

if [ -z $ANDROID_SDK_ROOT ]
then
  echo "Please set ANDROID_SDK_ROOT environment variable. e.g. \`export ANDROID_SDK_ROOT='<sdk-path>'\`"
  exit 1
fi

LOCAL_ENGINE=$1

if [ -z "$LOCAL_ENGINE" ]
then
  echo "Must specify a local engine. e.g. \`./run_android_tests.sh android_debug_unopt_x64\`"
  exit 1
fi

CLONE_FLUTTER=$ENGINE_PATH/src/flutter/tools/clone_flutter.sh
DART_TOOL_DIR=$ENGINE_PATH/src/flutter/.dart_tool

echo "$(date) START:CLONE_FLUTTER ------------------------------------------"
"$CLONE_FLUTTER" "$DART_TOOL_DIR"
echo "$(date) END:CLONE_FLUTTER ------------------------------------------"

set -e

FLUTTER_REPO=$ENGINE_PATH/src/flutter/.dart_tool/flutter
FLUTTER_TOOL=$FLUTTER_REPO/bin/flutter
SCENARIO_APP=$ENGINE_PATH/src/flutter/testing/scenario_app

if [ -z "$FLUTTER_TOOL" ]
then
  echo "Flutter tool not found. Try to look for $FLUTTER_TOOL."
  exit 1
fi

echo "Using Flutter tool: $FLUTTER_TOOL."

pushd "${BASH_SOURCE%/*}"
echo "$(date) START:PUB_GET ------------------------------------------"
"$FLUTTER_TOOL" pub get --offline --local-engine-src-path "$ENGINE_PATH/src" --local-engine="$LOCAL_ENGINE"
echo "$(date) END:PUB_GET ------------------------------------------"
popd

echo "$(date) START:WRITE_LOCAL_PROPERTIES ------------------------------------------"
cat > $SCENARIO_APP/android/local.properties << EOF
flutter.sdk=$FLUTTER_REPO
sdk.dir=$ANDROID_SDK_ROOT
EOF
echo "$(date) END:WRITE_LOCAL_PROPERTIES ------------------------------------------"

if [[ "$LOCAL_ENGINE" =~ debug ]]
then
  BUILD_MODE="debug"
elif [[ "$LOCAL_ENGINE" =~ profile ]]
then
  BUILD_MODE="profile"
elif [[ "$LOCAL_ENGINE" =~ release ]]
then
  BUILD_MODE="relase"
else
  echo "Couldn't determinate build mode from: $LOCAL_ENGINE"
  exit 1
fi

EMBEDDING_ARTIFACT_ID="flutter_embedding_$BUILD_MODE"

if [[ "$LOCAL_ENGINE" =~ x64 ]]
then
  LIB_FLUTTER_ARTIFACT_ID="x86_64_$BUILD_MODE"
  TARGET_PLATFORM="android-x64"
elif [[ "$LOCAL_ENGINE" =~ x86 ]]
then
  LIB_FLUTTER_ARTIFACT_ID="x86_$BUILD_MODE"
  TARGET_PLATFORM="android-x86"
elif [[ "$LOCAL_ENGINE" =~ arm64 ]]
then
  LIB_FLUTTER_ARTIFACT_ID="arm64_$BUILD_MODE"
  TARGET_PLATFORM="android-arm64"
elif [[ "$LOCAL_ENGINE" =~ arm ]]
then
  LIB_FLUTTER_ARTIFACT_ID="arm_$BUILD_MODE"
  TARGET_PLATFORM="android-arm"
else
  echo "Couldn't determinate arch from: $LOCAL_ENGINE"
  exit 1
fi

# This is `1.0.0-<engine-hash>`
FLUTTER_POM_VERSION=`sed -n -e 's/.*<version>\(.*\)<\/version>.*/\1/p' "$ENGINE_PATH/src/out/$LOCAL_ENGINE/$LIB_FLUTTER_ARTIFACT_ID.pom"`

# Delete the maven repo before exiting.
reboot() {
  echo "$(date) START:REMOVE_MAVEN_REPO ------------------------------------------"
  rm -rf "$SCENARIO_APP/.repo"
  echo "$(date) END:REMOVE_MAVEN_REPO --------------------------------------------"
}
trap reboot EXIT

# Create Maven repo, which is just symlinks to the files in the `out` directory.
echo "$(date) START:LOCAL_MAVEN_REPO ------------------------------------------"
mkdir -p "$SCENARIO_APP/.repo/io/flutter/$LIB_FLUTTER_ARTIFACT_ID/$FLUTTER_POM_VERSION/"
ln -s "$ENGINE_PATH/src/out/$LOCAL_ENGINE/$LIB_FLUTTER_ARTIFACT_ID.pom" "$SCENARIO_APP/.repo/io/flutter/$LIB_FLUTTER_ARTIFACT_ID/$FLUTTER_POM_VERSION/$LIB_FLUTTER_ARTIFACT_ID-$FLUTTER_POM_VERSION.pom"
ln -s "$ENGINE_PATH/src/out/$LOCAL_ENGINE/$LIB_FLUTTER_ARTIFACT_ID.jar" "$SCENARIO_APP/.repo/io/flutter/$LIB_FLUTTER_ARTIFACT_ID/$FLUTTER_POM_VERSION/$LIB_FLUTTER_ARTIFACT_ID-$FLUTTER_POM_VERSION.jar"

mkdir -p "$SCENARIO_APP/.repo/io/flutter/$EMBEDDING_ARTIFACT_ID/$FLUTTER_POM_VERSION/"
ln -s "$ENGINE_PATH/src/out/$LOCAL_ENGINE/$EMBEDDING_ARTIFACT_ID.pom" "$SCENARIO_APP/.repo/io/flutter/$EMBEDDING_ARTIFACT_ID/$FLUTTER_POM_VERSION/$EMBEDDING_ARTIFACT_ID-$FLUTTER_POM_VERSION.pom"
ln -s "$ENGINE_PATH/src/out/$LOCAL_ENGINE/$EMBEDDING_ARTIFACT_ID.jar" "$SCENARIO_APP/.repo/io/flutter/$EMBEDDING_ARTIFACT_ID/$FLUTTER_POM_VERSION/$EMBEDDING_ARTIFACT_ID-$FLUTTER_POM_VERSION.jar"
echo "$(date) END:LOCAL_MAVEN_REPO ------------------------------------------"

echo "$(date) START:TESTING ------------------------------------------"
pushd "${BASH_SOURCE%/*}/android"
./gradlew                                                   \
  assembleAndroidTest                                       \
  connectedAndroidTest                                      \
  -Plocal-engine-out="$ENGINE_PATH/src/out/$LOCAL_ENGINE"   \
  -Plocal-engine-repo="$SCENARIO_APP/.repo"                 \
  -Plocal-engine-build-mode="$BUILD_MODE"                   \
  -Ptarget-platform="$TARGET_PLATFORM"
popd
echo "$(date) END:TESTING ------------------------------------------"
