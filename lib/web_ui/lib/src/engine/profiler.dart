// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

typedef OnBenchmark = void Function(String name, num value);

/// The purpose of this class is to facilitate communication of
/// profiling/benchmark data to the outside world (e.g. a macrobenchmark that's
/// running a flutter app).
///
/// To use the [Profiler]:
///
/// 1. Enable the `FLUTTER_WEB_ENABLE_PROFILING` flag.
///
/// 2. Using js interop, assign a listener function to
///    `window._flutter_internal_on_benchmark` in the browser.
///
/// The listener function will be called every time a new benchmark number is
/// calculated. The signature is `Function(String name, num value)`.
class Profiler {
  Profiler._() {
    if (!isBenchmarkMode) {
      throw Exception(
        'Cannot use Profiler unless benchmark mode is enabled. '
        'You can enable it by using the FLUTTER_WEB_ENABLE_PROFILING flag.',
      );
    }
  }

  static bool isBenchmarkMode = const bool.fromEnvironment(
    'FLUTTER_WEB_ENABLE_PROFILING',
    defaultValue: true,
  );

  static Profiler ensureInitialized() {
    if (!isBenchmarkMode) {
      throw Exception(
        'Cannot use Profiler unless benchmark mode is enabled. '
        'You can enable it by using the FLUTTER_WEB_ENABLE_PROFILING flag.',
      );
    }
    if (Profiler._instance == null) {
      Profiler._instance = Profiler._();
    }
    return Profiler._instance;
  }

  static Profiler get instance {
    if (!isBenchmarkMode) {
      throw Exception(
        'Cannot use Profiler unless benchmark mode is enabled. '
        'You can enable it by using the FLUTTER_WEB_ENABLE_PROFILING flag.',
      );
    }
    if (_instance == null) {
      throw Exception(
        'Profiler has not been properly initialized. '
        'Make sure Profiler.ensureInitialized() is being called before you '
        'access Profiler.instance',
      );
    }
    return _instance;
  }

  static Profiler _instance;

  /// Used to send benchmark data to whoever is listening to them.
  void benchmark(String name, num value) {
    if (!isBenchmarkMode) {
      throw Exception(
        'Cannot use Profiler unless benchmark mode is enabled. '
        'You can enable it by using the FLUTTER_WEB_ENABLE_PROFILING flag.',
      );
    }

    final OnBenchmark onBenchmark =
        js_util.getProperty(html.window, '_flutter_internal_on_benchmark');
    if (onBenchmark != null) {
      onBenchmark(name, value);
    }
  }
}
