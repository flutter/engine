// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart';
import 'package:ui/src/engine.dart';
import 'package:test/test.dart';

import '../matchers.dart';
import '../mock_engine_canvas.dart';

const double kTolerance = 0.001;

void main() {
  group('PathMetric length', () {
    test('empty path', () {
      Path path = Path();
      expect(path.computeMetrics().isEmpty, isTrue);
    });

    test('simple line', () {
      Path path = Path();
      path.moveTo(100.0, 50.0);
      path.lineTo(200.0, 100.0);
      expect(path.computeMetrics().isEmpty, isFalse);
      final List<PathMetric> metrics = path.computeMetrics().toList();
      expect(metrics.length, 1);
      expect(metrics[0].length, within(distance: kTolerance, from: 111.803));
    });

    test('2 lines', () {
      Path path = Path();
      path.moveTo(100.0, 50.0);
      path.lineTo(200.0, 100.0);
      path.lineTo(200.0, 200.0);
      expect(path.computeMetrics().isEmpty, isFalse);
      final List<PathMetric> metrics = path.computeMetrics().toList();
      expect(metrics.length, 1);
      expect(metrics[0].length, within(distance: kTolerance, from: 211.803));
    });

    test('2 subpaths', () {
      Path path = Path();
      path.moveTo(100.0, 50.0);
      path.lineTo(200.0, 100.0);
      path.moveTo(200.0, 100.0);
      path.lineTo(200.0, 200.0);
      final List<double> contourLengths = computeLengths(path.computeMetrics());
      expect(contourLengths.length, 2);
      expect(contourLengths[0], within(distance: kTolerance, from: 111.803));
      expect(contourLengths[1], within(distance: kTolerance, from: 100.0));
    });
  });
}

List<double> computeLengths(PathMetrics pathMetrics) {
  final List<double> lengths = <double>[];
  for (PathMetric metric in pathMetrics) {
    lengths.add(metric.length);
  }
  return lengths;
}