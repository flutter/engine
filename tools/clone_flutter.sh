#!/bin/bash
set -e
set -x

if [[ "$CIRRUS_CI" = false || -z $CIRRUS_CI ]]
then
  echo "Cloning Flutter repo to local machine."
fi

if [[ -z $ENGINE_PATH ]]
then
  echo "Please set ENGINE_PATH environment variable."
  exit 1
fi

# Go to the engine git repo to get the date of the latest commit.
cd $ENGINE_PATH/src/flutter

# Special handling of release branches. We would like to run the tests against
# the release branch of flutter.
#
# This is a shortcut for the release branch, since we didn't address this part
# in LUCI yet.
ENGINE_BRANCH_NAME="flutter-1.20-candidate.7"

ON_RELEASE_BRANCH=true
echo "release branch $ENGINE_BRANCH_NAME"
echo "Engine on branch $ENGINE_BRANCH_NAME"

# Check if there is an argument added for repo location.
# If not use the location that should be set by Cirrus/LUCI.
FLUTTER_CLONE_REPO_PATH=$1

if [[ -z $FLUTTER_CLONE_REPO_PATH ]]
then
  if [[ -z $FRAMEWORK_PATH ]]
  then
    echo "Framework path should be set to run the script."
    exit 1
  fi
  # Do rest of the task in the root directory
  cd ~
  mkdir -p $FRAMEWORK_PATH
  cd $FRAMEWORK_PATH
else
  cd $FLUTTER_CLONE_REPO_PATH
fi

# Clone the Flutter Framework.
git clone https://github.com/flutter/flutter.git -b "$ENGINE_BRANCH_NAME"
cd flutter
