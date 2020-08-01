// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of dart.ui;

/// How the pointer has changed since the last report.
enum KeyChange {
  up,

  down,

  synchronize,

  cancel
}

class LogicalKeyData {
  const LogicalKeyData({
    required this.change,
    required this.key,
    this.character,
  });

  final KeyChange change;
  final int key;
  final String? character;
}

/// Information about the state of a pointer.
class KeyData {
  /// Creates an object that represents the state of a pointer.
  const KeyData({
    this.timeStamp,
    required this.change,
    required this.key,
    required this.logicalEvents,
  });

  final Duration? timeStamp;
  final KeyChange change;
  final int key;
  final List<LogicalKeyData> logicalEvents;

  @override
  String toString() => 'KeyData(timeStamp: $timeStamp, change: $change, key: $key)';

  /// Returns a complete textual description of the information in this object.
  String toStringFull() {
    return '$runtimeType('
           ')';
  }
}
