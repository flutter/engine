#!/bin/bash
#
# Copyright 2014 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Note: this script called from both ci/analyze.sh and from CIs outside this
# repo. If adding a new required argument to the script, please open an isssue
# at dart-lang/sdk to coordinate.

set -e

# Needed because if it is set, cd may print the path it changed to.
unset CDPATH

# Parse '--dart' into '$DART'.
# Parse '--flutter-dir' into '$FLUTTER_DIR'.
for argument in "$@"
do
  key=$(echo $argument | cut --fields 1 --delimiter='=')
  value=$(echo $argument | cut --fields 2 --delimiter='=')

  case "$key" in
    "dart")           DART="$value" ;;
    "flutter-dir")    FLUTTER_DIR="$value" ;;
    *)
  esac
done

echo "Using dart from $DART_BIN"
"$DART" --version
echo ""

"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/ci"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/flutter_frontend_server"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/impeller/golden_tests_harvester"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/impeller/tessellator/dart"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/lib/gpu"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/lib/ui"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/testing"
"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/tools"

echo ""

# Check that dart libraries conform.
echo "Checking the integrity of the Web SDK"
(cd "$FLUTTER_DIR/web_sdk"; "$DART" pub get)
(cd "$FLUTTER_DIR/web_sdk/web_test_utils"; "$DART" pub get)
(cd "$FLUTTER_DIR/web_sdk/web_engine_tester"; "$DART" pub get)

"$DART" analyze --fatal-infos --fatal-warnings "$FLUTTER_DIR/web_sdk"

WEB_SDK_TEST_FILES="$FLUTTER_DIR/web_sdk/test/*"
for testFile in $WEB_SDK_TEST_FILES
do
  echo "Running $testFile"
  (cd "$FLUTTER_DIR"; FLUTTER_DIR="$FLUTTER_DIR" "$DART" --enable-asserts $testFile)
done
