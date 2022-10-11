// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// Converts an integer into a hex string with the given number of digits.
String toHex(int? value, {int digits = 8}) {
  if (value == null) {
    return 'null';
  }
  return '0x${value.toRadixString(16).padLeft(digits, '0')}';
}
