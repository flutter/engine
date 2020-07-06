// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9

// HACK: pretend to be dart.ui in order to access its internals
library dart.ui;

import 'dart:convert' show json;

import 'package:test/test.dart';

// HACK: these parts are to get access to private functions tested here.
part '../../lib/ui/pointer.dart';

void main() {
  group('PointerData test', () {
    test('Test serialization and deserialization to/from JSON', () async {
      void compare(final PointerData original, final PointerData restored) {
        expect(restored.timeStamp, original.timeStamp);
        expect(restored.change, original.change);
        expect(restored.kind, original.kind);
        expect(restored.signalKind, original.signalKind);
        expect(restored.device, original.device);
        expect(restored.pointerIdentifier, original.pointerIdentifier);
        expect(restored.physicalX, original.physicalX);
        expect(restored.physicalY, original.physicalY);
        expect(restored.physicalDeltaX, original.physicalDeltaX);
        expect(restored.physicalDeltaY, original.physicalDeltaY);
        expect(restored.buttons, original.buttons);
        expect(restored.obscured, original.obscured);
        expect(restored.synthesized, original.synthesized);
        expect(restored.pressure, original.pressure);
        expect(restored.pressureMin, original.pressureMin);
        expect(restored.pressureMax, original.pressureMax);
        expect(restored.distance, original.distance);
        expect(restored.distanceMax, original.distanceMax);
        expect(restored.size, original.size);
        expect(restored.radiusMajor, original.radiusMajor);
        expect(restored.radiusMinor, original.radiusMinor);
        expect(restored.radiusMin, original.radiusMin);
        expect(restored.radiusMax, original.radiusMax);
        expect(restored.orientation, original.orientation);
        expect(restored.tilt, original.tilt);
        expect(restored.platformData, original.platformData);
        expect(restored.scrollDeltaX, original.scrollDeltaX);
        expect(restored.scrollDeltaY, original.scrollDeltaY);
      }

      const PointerData defaultData = PointerData();
      final String defaultDataString = json.encode(defaultData);
      expect(defaultDataString, '{}');
      compare(defaultData, PointerData.fromJson(defaultDataString));

      const PointerData customizeData = PointerData(
        timeStamp: Duration(hours: 1),
        change: PointerChange.move,
        kind: PointerDeviceKind.invertedStylus,
        signalKind: PointerSignalKind.scroll,
        device: 3,
        pointerIdentifier: 42,
        physicalX: 3.14,
        physicalY: 2.718,
        physicalDeltaX: 1.414,
        physicalDeltaY: 1.732,
        buttons: 4,
        obscured: true,
        synthesized: true,
        pressure: 101.325,
        pressureMin: 0.132,
        pressureMax: 760.0,
        distance: 1.609,
        distanceMax: 3.28,
        size: 0.618,
        radiusMajor: 1.123,
        radiusMinor: 4.567,
        orientation: 1.257,
        tilt: 0.628,
        platformData: 137,
        scrollDeltaX: 273.15,
        scrollDeltaY: 195.42
      );
      compare(customizeData, PointerData.fromJson(json.encode(customizeData)));
    });
  });
}
