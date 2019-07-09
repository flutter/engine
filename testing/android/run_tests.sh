#!/bin/bash

pushd android_host_app/android

./gradlew \
  -Pverbose=true \
  -Ptarget=$(pwd)/../test/hello.dart \
  -Ptrack-widget-creation=false \
  -Pfilesystem-scheme=org-dartlang-root \
  connectedAndroidTest

# To use Firebase Test Lab:
# assembleAndroidTest

popd