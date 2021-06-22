// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'hash_codes.dart';

abstract class XUnion {
  const XUnion();

  int get $ordinal;
  Object get $data;

  @override
  int get hashCode => deepHash([$ordinal, $data]);

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other)) {
      return true;
    }
    if (runtimeType != other.runtimeType) {
      return false;
    }
    final XUnion otherUnion = other;
    if ($ordinal != otherUnion.$ordinal) {
      return false;
    }
    return deepEquals($data, otherUnion.$data);
  }
}

typedef XUnionFactory<T> = T Function(int index, Object data);
