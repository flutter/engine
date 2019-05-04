// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
class Versions {
  /// Stub implementation. See docs in `../ui/`.
  Versions._(
    this.dartVersion,
    this.skiaVersion,
    this.flutterEngineVersion
  ) : assert(dartVersion != null),
      assert(skiaVersion != null),
      assert(flutterEngineVersion != null);

  final String dartVersion;
  final String skiaVersion;
  final String flutterEngineVersion;
}

/// Stub implementation. See docs in `../ui/`.
final Versions versions = Versions._('', '', '');
