#!/bin/bash
# Copyright 2018 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Is meant to be run in the build directory.
readonly SCRIPT_ROOT="$(cd $(dirname ${BASH_SOURCE[0]} ) && pwd)"
readonly DATA_DIR="$SCRIPT_ROOT/test_data/extract_zircon_constants"

echo Testing extract-zircon-constants.py
exec python "$DATA_DIR/extract-zircon-constants.py" \
  --dry-run \
  --errors "$DATA_DIR/errors.h" \
  --rights "$DATA_DIR/rights.h" \
  --types "$DATA_DIR/types.h" \
  --dartfmt "$DATA_DIR/dartfmt" \
  --dart-constants "$DATA_DIR/constants.dart"

