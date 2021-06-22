// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:typed_data';

import 'package:zircon/zircon.dart';

import 'hash_codes.dart';

/// UnknownRawData is a container for the raw bytes and handles of an unknown
/// envelope. It has an associate UnknownRawDataType that allows encoding/
/// decoding instances of this class.
class UnknownRawData {
  Uint8List data;
  List<Handle> handles;
  UnknownRawData(this.data, this.handles);

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is UnknownRawData &&
          deepEquals(data, other.data) &&
          deepEquals(handles, other.handles);

  @override
  int get hashCode => deepHash([data, handles]);

  @override
  String toString() {
    return 'data: $data, handles: $handles';
  }

  /// Attempts to close all stored handles, ignoring any errors.
  void closeHandles() {
    for (final handle in handles) {
      try {
        handle.close();
        // ignore: avoid_catches_without_on_clauses
      } catch (e) {
        // best effort
      }
    }
  }
}
