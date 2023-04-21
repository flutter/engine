// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'fake_asset_manager.dart';

Future<void>? _platformInitializedFuture;

Future<void> initializeTestFlutterViewEmbedder({double devicePixelRatio = 3.0}) {
  // Force-initialize FlutterViewEmbedder so it doesn't overwrite test pixel ratio.
  ensureFlutterViewEmbedderInitialized();

  // The following parameters are hard-coded in Flutter's test embedder. Since
  // we don't have an embedder yet this is the lowest-most layer we can put
  // this stuff in.
  window.debugOverrideDevicePixelRatio(devicePixelRatio);
  window.webOnlyDebugPhysicalSizeOverride =
      ui.Size(800 * devicePixelRatio, 600 * devicePixelRatio);
  scheduleFrameCallback = () {};
  ui.debugEmulateFlutterTesterEnvironment = true;

  // Initialize platform once and reuse across all tests.
  if (_platformInitializedFuture != null) {
    return _platformInitializedFuture!;
  }
  return _platformInitializedFuture =
      initializeEngine(assetManager: fakeAssetManager);
}
