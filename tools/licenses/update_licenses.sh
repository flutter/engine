#!/bin/bash

# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

# The licenses script can only be run from the the project directory.
cd flutter/tools/licenses

# Remove output of old run if any.
rm -rf ../../../out/license_script_output

# Update the license script project dependencies.
pub get

# Regenerate the golden file.
dart --checked lib/main.dart --src ../../.. --out ../../../out/license_script_output --golden ../../travis/licenses_golden

# Replace the golden file.
cp ../../../out/license_script_output/* ../../travis/licenses_golden/
