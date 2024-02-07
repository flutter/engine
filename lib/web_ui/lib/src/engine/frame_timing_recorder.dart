// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

class FrameTimingRecorder {
  int? _vsyncStartMicros;
  int? _buildStartMicros;
  int? _buildFinishMicros;
  int? _rasterStartMicros;
  int? _rasterFinishMicros;

  /// Collects frame timings from frames.
  ///
  /// This list is periodically reported to the framework (see [_kFrameTimingsSubmitInterval]).
  static List<ui.FrameTiming> _frameTimings = <ui.FrameTiming>[];

  static void startFrame() {
    _currentFrameTimingRecorder = frameTimingsEnabled
      ? FrameTimingRecorder()
      : null;
  }

  static FrameTimingRecorder? _currentFrameTimingRecorder;
  static FrameTimingRecorder? get currentRecorder => _currentFrameTimingRecorder;

  /// The last time (in microseconds) we submitted frame timings.
  static int _frameTimingsLastSubmitTime = _nowMicros();
  /// The amount of time in microseconds we wait between submitting
  /// frame timings.
  static const int _kFrameTimingsSubmitInterval = 100000; // 100 milliseconds

  /// Whether we are collecting [ui.FrameTiming]s.
  static bool get frameTimingsEnabled {
    return EnginePlatformDispatcher.instance.onReportTimings != null;
  }

  /// Current timestamp in microseconds taken from the high-precision
  /// monotonically increasing timer.
  ///
  /// See also:
  ///
  /// * https://developer.mozilla.org/en-US/docs/Web/API/Performance/now,
  ///   particularly notes about Firefox rounding to 1ms for security reasons,
  ///   which can be bypassed in tests by setting certain browser options.
  static int _nowMicros() {
    return (domWindow.performance.now() * 1000).toInt();
  }

  void recordVsyncStart([int? vsyncStart]) {
    assert(_vsyncStartMicros == null, "can't record vsync start more than once");
    _vsyncStartMicros = vsyncStart ?? _nowMicros();
  }

  void recordBuildStart([int? buildStart]) {
    assert(_buildStartMicros == null, "can't record build start more than once");
    _buildStartMicros = buildStart ?? _nowMicros();
  }

  void recordBuildFinish([int? buildFinish]) {
    assert(_buildFinishMicros == null, "can't record build finish more than once");
    _buildFinishMicros = buildFinish ?? _nowMicros();
  }

  void recordRasterStart([int? rasterStart]) {
    assert(_rasterStartMicros == null, "can't record raster start more than once");
    _rasterStartMicros = rasterStart ?? _nowMicros();
  }

  void recordRasterFinish([int? rasterFinish]) {
    assert(_rasterFinishMicros == null, "can't record raster finish more than once");
    _rasterFinishMicros = rasterFinish ?? _nowMicros();
  }

  void submitTimings() {
    assert(
      _buildStartMicros != null &&
      _buildFinishMicros != null &&
      _rasterStartMicros != null &&
      _rasterFinishMicros != null,
      'Attempted to submit an incomplete timings.'
    );
    final ui.FrameTiming timing = ui.FrameTiming(
      vsyncStart: _vsyncStartMicros!,
      buildStart: _buildStartMicros!,
      buildFinish: _buildFinishMicros!,
      rasterStart: _rasterStartMicros!,
      rasterFinish: _rasterFinishMicros!,
      rasterFinishWallTime: _rasterFinishMicros!,
    );
    _frameTimings.add(timing);
    final int now = _nowMicros();
    if (now - _frameTimingsLastSubmitTime > _kFrameTimingsSubmitInterval) {
      _frameTimingsLastSubmitTime = now;
      EnginePlatformDispatcher.instance.invokeOnReportTimings(_frameTimings);
      _frameTimings = <ui.FrameTiming>[];
    }
  }
}
