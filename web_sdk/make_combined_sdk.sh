#!/bin/bash

# Reference script for creating flutter analyzer summmary locally.

# Copy dartdevc patched sources into temp_dart_sdk.
mkdir -p temp_dart_sdk/lib
cp -RL gen/third_party/dart/utils/dartdevc/patched_sdk/lib temp_dart_sdk/

# Copy stub ui dart sources into temp_dart_sdk.
mkdir -p temp_dart_sdk/lib/ui
cp -RL flutter_web_sdk/lib/ui/ temp_dart_sdk/lib/ui

# Copy libraries.dart into temp_dart_sdk.
# NOTE: THERE ARE TWO?
mkdir -p temp_dart_sdk/lib/_internal
cp -RL ../../flutter/web_sdk/libraries.dart temp_dart_sdk/lib/_internal/libraries.dart
cp -RL ../../flutter/web_sdk/libraries.dart temp_dart_sdk/lib/_internal/sdk_library_metadata/lib/libraries.dart

# Build summary file.
# third_party/dart/pkg/analyzer/tool/summary/build_sdk_summaries.dart
./dart "$BUILD_SDK_SUMMARY_TOOL" build-strong temp_dart_sdk/lib/_internal/strong.sum temp_dart_sdk/