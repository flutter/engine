#!/bin/bash
set -e


echo "Framework path: $FRAMEWORK_PATH"

# Go to the engine git repo to get the date of the latest commit.
cd $ENGINE_PATH/src/flutter
# Get latest commit's time for the engine repo.
# Use date based on local time otherwise timezones might get mixed.
LATEST_COMMIT_TIME_ENGINE=`git log -1 --date=local --format="%ad"`
echo "Latest commit time on engine found as $LATEST_COMMIT_TIME_ENGINE"

# Do rest of the task in the root directory
cd ~
mkdir -p $FRAMEWORK_PATH
cd $FRAMEWORK_PATH
# Clone the Flutter Framework.
git clone https://github.com/flutter/flutter.git
cd flutter
# Get the time of the youngest commit older than engine commit.
# Before makes the comparison considering the timezone as well.
COMMIT_NO=`git log --before="$LATEST_COMMIT_TIME_ENGINE" -n 1 | grep commit | cut -d ' ' -f2`
echo "Using the flutter/flutter commit $COMMIT_NO";
git reset --hard $COMMIT_NO
