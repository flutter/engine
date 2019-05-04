// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
enum PointerChange {
  /// Stub implementation. See docs in `../ui/`.
  cancel,

  /// Stub implementation. See docs in `../ui/`.
  add,

  /// Stub implementation. See docs in `../ui/`.
  remove,

  /// Stub implementation. See docs in `../ui/`.
  hover,

  /// Stub implementation. See docs in `../ui/`.
  down,

  /// Stub implementation. See docs in `../ui/`.
  move,

  /// Stub implementation. See docs in `../ui/`.
  up,
}

/// Stub implementation. See docs in `../ui/`.
enum PointerDeviceKind {
  /// Stub implementation. See docs in `../ui/`.
  touch,

  /// Stub implementation. See docs in `../ui/`.
  mouse,

  /// Stub implementation. See docs in `../ui/`.
  stylus,

  /// Stub implementation. See docs in `../ui/`.
  invertedStylus,

  /// Stub implementation. See docs in `../ui/`.
  unknown
}

/// Stub implementation. See docs in `../ui/`.
enum PointerSignalKind {
  /// Stub implementation. See docs in `../ui/`.
  none,

  /// Stub implementation. See docs in `../ui/`.
  scroll,

  /// Stub implementation. See docs in `../ui/`.
  unknown
}

/// Stub implementation. See docs in `../ui/`.
class PointerData {
  /// Stub implementation. See docs in `../ui/`.
  const PointerData({
    this.timeStamp: Duration.zero,
    this.change: PointerChange.cancel,
    this.kind: PointerDeviceKind.touch,
    this.signalKind,
    this.device: 0,
    this.physicalX: 0.0,
    this.physicalY: 0.0,
    this.buttons: 0,
    this.obscured: false,
    this.pressure: 0.0,
    this.pressureMin: 0.0,
    this.pressureMax: 0.0,
    this.distance: 0.0,
    this.distanceMax: 0.0,
    this.size: 0.0,
    this.radiusMajor: 0.0,
    this.radiusMinor: 0.0,
    this.radiusMin: 0.0,
    this.radiusMax: 0.0,
    this.orientation: 0.0,
    this.tilt: 0.0,
    this.platformData: 0,
    this.scrollDeltaX: 0.0,
    this.scrollDeltaY: 0.0,
  });

  /// Stub implementation. See docs in `../ui/`.
  final Duration timeStamp;

  /// Stub implementation. See docs in `../ui/`.
  final PointerChange change;

  /// Stub implementation. See docs in `../ui/`.
  final PointerDeviceKind kind;

  /// Stub implementation. See docs in `../ui/`.
  final PointerSignalKind signalKind;

  /// Stub implementation. See docs in `../ui/`.
  final int device;

  /// Stub implementation. See docs in `../ui/`.
  final double physicalX;

  /// Stub implementation. See docs in `../ui/`.
  final double physicalY;

  /// Stub implementation. See docs in `../ui/`.
  final int buttons;

  /// Stub implementation. See docs in `../ui/`.
  final bool obscured;

  /// Stub implementation. See docs in `../ui/`.
  final double pressure;

  /// Stub implementation. See docs in `../ui/`.
  final double pressureMin;

  /// Stub implementation. See docs in `../ui/`.
  final double pressureMax;

  /// Stub implementation. See docs in `../ui/`.
  final double distance;

  /// Stub implementation. See docs in `../ui/`.
  final double distanceMax;

  /// Stub implementation. See docs in `../ui/`.
  final double size;

  /// Stub implementation. See docs in `../ui/`.
  final double radiusMajor;

  /// Stub implementation. See docs in `../ui/`.
  final double radiusMinor;

  /// Stub implementation. See docs in `../ui/`.
  final double radiusMin;

  /// Stub implementation. See docs in `../ui/`.
  final double radiusMax;

  /// Stub implementation. See docs in `../ui/`.
  final double orientation;

  /// Stub implementation. See docs in `../ui/`.
  final double tilt;

  /// Stub implementation. See docs in `../ui/`.
  final int platformData;

  /// Stub implementation. See docs in `../ui/`.
  final double scrollDeltaX;

  /// Stub implementation. See docs in `../ui/`.
  final double scrollDeltaY;

  @override
  String toString() => '$runtimeType(x: $physicalX, y: $physicalY)';

  /// Stub implementation. See docs in `../ui/`.
  String toStringFull() {
    return '$runtimeType('
             'timeStamp: $timeStamp, '
             'change: $change, '
             'kind: $kind, '
             'signalKind: $signalKind, '
             'device: $device, '
             'physicalX: $physicalX, '
             'physicalY: $physicalY, '
             'buttons: $buttons, '
             'pressure: $pressure, '
             'pressureMin: $pressureMin, '
             'pressureMax: $pressureMax, '
             'distance: $distance, '
             'distanceMax: $distanceMax, '
             'size: $size, '
             'radiusMajor: $radiusMajor, '
             'radiusMinor: $radiusMinor, '
             'radiusMin: $radiusMin, '
             'radiusMax: $radiusMax, '
             'orientation: $orientation, '
             'tilt: $tilt, '
             'platformData: $platformData, '
             'scrollDeltaX: $scrollDeltaX, '
             'scrollDeltaY: $scrollDeltaY'
           ')';
  }
}

/// Stub implementation. See docs in `../ui/`.
class PointerDataPacket {
  /// Stub implementation. See docs in `../ui/`.
  const PointerDataPacket({ this.data: const <PointerData>[] });

  /// Stub implementation. See docs in `../ui/`.
  final List<PointerData> data;
}
