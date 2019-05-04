// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
class IsolateNameServer {
  // This class is only a namespace, and should not be instantiated or
  // extended directly.
  factory IsolateNameServer._() => null;

  /// Stub implementation. See docs in `../ui/`.
  static SendPort lookupPortByName(String name) {
    assert(name != null, "'name' cannot be null.");
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static bool registerPortWithName(SendPort port, String name) {
    assert(port != null, "'port' cannot be null.");
    assert(name != null, "'name' cannot be null.");
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  static bool removePortNameMapping(String name) {
    assert(name != null, "'name' cannot be null.");
    throw UnimplementedError();
  }
}
