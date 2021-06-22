// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

// Bits represents a bit field, i.e. no bit set, one bit set, or multiple bit
// set.
abstract class Bits {
  const Bits();

  int get $value;

  @override
  bool operator ==(dynamic other) {
    if (other is Bits) {
      return $value == other.$value;
    }
    return false;
  }

  @override
  int get hashCode => $value.hashCode;

  bool hasUnknownBits();

  int getUnknownBits();
}

typedef BitsFactory<T> = T Function(int value);
