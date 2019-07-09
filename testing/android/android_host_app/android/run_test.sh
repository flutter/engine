#!/bin/bash

./gradlew \
        -Pverbose=true \
        -Ptarget=$(pwd)/../test/hello.dart \
        -Ptrack-widget-creation=false \
        -Pfilesystem-scheme=org-dartlang-root \
        connectedAndroidTest       

#assembleAndroidTest

