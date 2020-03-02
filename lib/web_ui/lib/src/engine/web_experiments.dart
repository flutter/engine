// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

const MethodCodec codec = JSONMethodCodec();

final WebExperiments webExperiments = WebExperiments._();

/// A bag of all experiment flags in the web engine.
///
/// This class also handles platform messages that can be sent to enable/disable
/// certain experiments at runtime without the need to access engine internals.
class WebExperiments {
  WebExperiments._();

  /// Experiment flag for using the Skia-based rendering backend.
  bool get useSkia => _useSkia ?? false;
  set useSkia(bool enabled) {
    _useSkia = enabled;
  }
  bool _useSkia = const bool.fromEnvironment('FLUTTER_WEB_USE_SKIA');

  /// Experiment flag for using canvas-based text measurement.
  bool get useCanvasText => _useCanvasText ?? false;
  set useCanvasText(bool enabled) {
    _useCanvasText = enabled;
  }
  bool _useCanvasText =
      const bool.fromEnvironment('FLUTTER_WEB_USE_EXPERIMENTAL_CANVAS_TEXT');

  /// Reset all experimental flags to their default values.
  void reset() {
    _useSkia = null;
    _useCanvasText = null;
  }

  /// Handles the platform message used to enable/disable web experiments in the
  /// web engine.
  void enableWebExperiments(Map<String, dynamic> message) {
    for (final String name in message.keys) {
      final bool enabled = message[name];
      switch (name) {
        case 'useSkia':
          _useSkia = enabled;
          break;
        case 'useCanvasText':
          _useCanvasText = enabled;
          break;
      }
    }
  }
}
