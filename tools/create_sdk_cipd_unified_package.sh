#!/bin/bash

# This script requires depot_tools to be on path.

print_usage () {
  echo "Usage: create_sdk_cipd_united_package.sh <PATH_TO_SDK_DIR> <VERSION_TAG>"
  echo "  where:"
  echo "    - PATH_TO_SDK_DIR is the path to the sdk folder"
  echo "    - VERSION_TAG is the version of the package, e.g. 28r6 or 31v1"
  echo ""
  echo "This script downloads the specified packages and uploads them to CIPD. You may need"
  echo "to accept licences during the download process."
  echo ""
  echo "Manage the packages to download in 'android_sdk_packages.txt'. You can use"
  echo "'sdkmanager --list --include_obsolete' in cmdline-tools to list all available packages"
  echo ""
  echo "This script expects sdkmanager to be available at '<PATH_TO_SDK_DIR>/cmdline-tools/latest/bin'"
  echo "and should only be run on linux or macos hosts."
  echo ""
  echo "For more see: https://developer.android.com/studio/command-line/sdkmanager"
}

# Validate version is provided
if [[ $2 == "" ]]; then
  print_usage
  exit 1
fi

# Validate directory contains all SDK packages
if [[ ! -d "$1" ]]; then
  echo "Directory $1 not found."
  print_usage
  exit 1
fi
if [[ ! -d "$1/cmdline-tools" ]]; then
  echo "SDK directory does not contain $1/cmdline-tools."
  print_usage
  exit 1
fi

platforms=("linux" "macosx" "windows")
upload_dirs=("platform-tools" "build-tools" "platforms" "tools" "cmdline-tools")
sdkmanager_path="$1/cmdline-tools/latest/bin/sdkmanager"
temp_dir="$1/temp"
mkdir $temp_dir
for platform in "${platforms[@]}"; do
  sdk_root="$temp_dir/sdk_$platform"
  echo "Creating temporary working directory for $platform: $sdk_root"
  mkdir $sdk_root
  for package in $(cat android_sdk_packages.txt); do
    echo "Installing $package"
    REPO_OS_OVERRIDE=$platform $sdkmanager_path --sdk_root=$sdk_root $package
  done
  echo "Consolidating files for upload"
  upload_dir="$sdk_root/upload"
  mkdir $upload_dir
  for dir in "${upload_dirs[@]}"; do
    cp -r "$sdk_root/$dir" "$upload_dir"
  done
  cipd_name="$platform-amd64"
  if [[ $platform == "macosx" ]]; then
    cipd_name="mac-amd64"
  fi
  echo "Uploading $cipd_name to CIPD"
  cipd create -in $upload_dir -name "flutter/android/sdk/all/$cipd_name" -install-mode copy -tag version:$2
  rm -rf $sdk_root
done
