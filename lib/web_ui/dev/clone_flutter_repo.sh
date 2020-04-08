#!/bin/bash
set -e
set -x

FLUTTER_CLONE_REPO=$1

if [[ -z $FLUTTER_CLONE_REPO ]]
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
  cd $FLUTTER_CLONE_REPO
fi


# Clone the Flutter Framework.
git clone https://github.com/flutter/flutter.git
cd flutter
