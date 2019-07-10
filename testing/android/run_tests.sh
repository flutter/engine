#!/bin/bash

pushd android_host_app

# Required to create gradlew script
if [ "$FRAMEWORK_PATH" = "" ]
then
  FLUTTER_BIN=flutter
else
  FLUTTER_BIN=$FRAMEWORK_PATH/bin/flutter
fi
# Creates android/gradlew
$FLUTTER_BIN build apk --debug

pushd android

case $1 in
  "--firebase" )  # Use Firebase Test Lab
    ./gradlew \
      -Pverbose=true \
      -Ptrack-widget-creation=false \
      -Pfilesystem-scheme=org-dartlang-root \
      assembleAndroidTest
    echo $GCLOUD_FIREBASE_TESTLAB_KEY > ${HOME}/gcloud-service-key.json
    gcloud auth activate-service-account --key-file=${HOME}/gcloud-service-key.json
    gcloud --quiet config set project flutter-infra
    gcloud firebase test android run --type instrumentation \
      --app build/app/outputs/apk/app.apk \
      --test build/app/outputs/apk/androidTest/debug/app-debug-androidTest.apk\
      --timeout 2m \
      --results-bucket=gs://flutter_firebase_testlab \
      --results-dir=engine_android_test/$GIT_REVISION/$CIRRUS_BUILD_ID
    # Check logcat for "E/flutter" - if it's there, something's wrong.
    gsutil cp gs://flutter_firebase_testlab/release_smoke_test/$GIT_REVISION/$CIRRUS_BUILD_ID/walleye-26-en-portrait/logcat /tmp/logcat
    ! grep "E/flutter" /tmp/logcat || false
    grep "I/flutter" /tmp/logcat;;
  *)  # Use local device instead
    ./gradlew \
      -Pverbose=true \
      -Ptrack-widget-creation=false \
      -Pfilesystem-scheme=org-dartlang-root \
      connectedAndroidTest;;
esac

popd