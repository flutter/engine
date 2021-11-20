#!/bin/bash

# This script requires depot_tools to be on path.

print_usage () {
  echo "Usage: create_sdk_cipd_united_package.sh <PATH_TO_SDK_DIR> <VERSION_TAG>"
  echo "  where:"
  echo "    - PATH_TO_SDK_DIR is the path to the sdk folder"
  echo "    - VERSION_TAG is the version of the package, e.g. 28r6 or 31v1"
  echo ""
  echo "This script downloads the packages specified in android_sdk_packages.txt and uploads"
  echo "them to CIPD for linux, mac, and windows."
  echo ""
  echo "Manage the packages to download in 'android_sdk_packages.txt'. You can use"
  echo "'sdkmanager --list --include_obsolete' in cmdline-tools to list all available packages."
  echo "Packages should be listed in the format of <package-name>:<directory-to-upload>."
  echo "For example, build-tools;31.0.0:build-tools"
  echo "Multiple directories to upload can be specified by delimiting by additional ':'"
  echo ""
  echo "This script expects the cmdline-tools to be installed in your specified PATH_TO_SDK_DIR"
  echo "and should only be run on linux or macos hosts."
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

# Find the sdkmanager in cmdline-tools. We default to using latest if available.
sdkmanager_path="$1/cmdline-tools/latest/bin/sdkmanager"
find_results=()
while IFS= read -r line; do
  find_results+=("$line")
done < <(find "$1/cmdline-tools" -name sdkmanager)
i=0
while [ ! -f "$sdkmanager_path" ]; do
  if [ $i -ge ${#find_results[@]} ]; then
    echo "Unable to find sdkmanager in the SDK directory. Please ensure cmdline-tools is installed."
    exit 1
  fi
  sdkmanager_path="${find_results[$i]}"
  echo $sdkmanager_path
  ((i++))
done

# We create a new temporary SDK directory because the default working directory
# tends to not update/redownload packages if they are being used. This guarantees
# a clean install of Android SDK.
temp_dir="$1/temp_cipd_android_sdk"
rm -rf $temp_dir
mkdir $temp_dir

for platform in "${platforms[@]}"; do
  sdk_root="$temp_dir/sdk_$platform"
  upload_dir="$sdk_root/upload"
  echo "Creating temporary working directory for $platform: $sdk_root"
  mkdir $sdk_root
  mkdir $upload_dir

  # Download all the packages with sdkmanager.
  for package in $(cat android_sdk_packages.txt); do
    split=(${package//:/ })
    echo "Installing ${split[0]}"
    REPO_OS_OVERRIDE=$platform yes "y" | $sdkmanager_path --sdk_root=$sdk_root ${split[0]}
    # We copy only the relevant directories to a temporary dir
    # for upload. sdkmanager creates extra files that we don't need.
    array_length=${#split[@]}
    for (( i=1; i<${array_length}; i++ )); do
      cp -r "$sdk_root/${split[$i]}" "$upload_dir"
    done
  done

  # Mac uses a different sdkmanager name than the platform name used in gn.
  cipd_name="$platform-amd64"
  if [[ $platform == "macosx" ]]; then
    cipd_name="mac-amd64"
  fi
  echo "Uploading $cipd_name to CIPD"
  cipd create -in $upload_dir -name "flutter/android/sdk/all/$cipd_name" -install-mode copy -tag version:$2

  rm -rf $sdk_root
done
rm -rf $temp_dir
