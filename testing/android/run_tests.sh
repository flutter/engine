#!/bin/bash

pushd android_host_app

# Required to create gradlew script
flutter build apk

pushd android

./gradlew \
  -Pverbose=true \
  -Ptrack-widget-creation=false \
  -Pfilesystem-scheme=org-dartlang-root \
  connectedAndroidTest

# TODO(jackson) use Firebase Test Lab:
# assembleAndroidTest

popd