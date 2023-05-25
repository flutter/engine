// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection';
import 'dart:ffi';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmPathMetrics extends IterableBase<ui.PathMetric>
    implements ui.PathMetrics {
  SkwasmPathMetrics({required this.path, required this.forceClosed});

  SkwasmPath path;
  bool forceClosed;

  @override
  late Iterator<ui.PathMetric> iterator = SkwasmPathMetricIterator(path, forceClosed);
}

class SkwasmPathMetricIterator implements SkwasmObjectWrapper<RawContourMeasureIter>, Iterator<ui.PathMetric> {
  SkwasmPathMetricIterator(SkwasmPath path, bool forceClosed)
      : handle = contourMeasureIterCreate(path.handle, forceClosed, 1.0) {
    _registry.register(this);
  }

  static final SkwasmFinalizationRegistry<RawContourMeasureIter> _registry =
    SkwasmFinalizationRegistry<RawContourMeasureIter>(contourMeasureIterDispose);

  @override
  final ContourMeasureIterHandle handle;
  bool _isDisposed = false;
  SkwasmPathMetric? _current;
  int _nextIndex = 0;

  void dispose() {
    assert(!_isDisposed);
    _registry.unregister(this);
    contourMeasureIterDispose(handle);
    _isDisposed = true;
  }

  @override
  ui.PathMetric get current {
    if (_current == null) {
      throw RangeError(
        'PathMetricIterator is not pointing to a PathMetric. This can happen in two situations:\n'
        '- The iteration has not started yet. If so, call "moveNext" to start iteration.\n'
        '- The iterator ran out of elements. If so, check that "moveNext" returns true prior to calling "current".');
    }
    return _current!;
  }

  @override
  bool moveNext() {
    final ContourMeasureHandle measureHandle = contourMeasureIterNext(handle);
    if (measureHandle == nullptr) {
      _current = null;
      return false;
    } else {
      _current = SkwasmPathMetric(measureHandle, _nextIndex);
      _nextIndex++;
      return true;
    }
  }
}

class SkwasmPathMetric implements SkwasmObjectWrapper<RawContourMeasure>, ui.PathMetric {
  SkwasmPathMetric(this.handle, this.contourIndex) {
    _registry.register(this);
  }

  static final SkwasmFinalizationRegistry<RawContourMeasure> _registry =
    SkwasmFinalizationRegistry<RawContourMeasure>(contourMeasureDispose);

  @override
  final ContourMeasureHandle handle;
  bool _isDisposed = false;

  void dispose() {
    assert(!_isDisposed);
    _registry.unregister(this);
    contourMeasureDispose(handle);
    _isDisposed = true;
  }

  @override
  final int contourIndex;

  @override
  ui.Path extractPath(double start, double end, {bool startWithMoveTo = true}) {
    return SkwasmPath.fromHandle(
        contourMeasureGetSegment(handle, start, end, startWithMoveTo));
  }

  @override
  ui.Tangent? getTangentForOffset(double distance) {
    return withStackScope((StackScope scope) {
      final Pointer<Float> outPosition = scope.allocFloatArray(4);
      final Pointer<Float> outTangent =
          Pointer<Float>.fromAddress(outPosition.address + sizeOf<Float>() * 2);
      final bool result =
          contourMeasureGetPosTan(handle, distance, outPosition, outTangent);
      assert(result);
      return ui.Tangent(
        ui.Offset(outPosition[0], outPosition[1]),
        ui.Offset(outTangent[0], outTangent[1])
      );
    });
  }

  @override
  bool get isClosed => contourMeasureIsClosed(handle);

  @override
  double get length => contourMeasureLength(handle);
}
