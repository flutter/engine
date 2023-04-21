// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/test.dart';
import 'package:ui/src/engine.dart' as engine;
import 'package:ui/ui.dart' as ui;

import 'fake_asset_manager.dart';

void setUpUnitTests() {
  late final FakeAssetScope debugFontsScope;
  setUpAll(() async {
    // Force-initialize FlutterViewEmbedder so it doesn't overwrite test pixel ratio.
    engine.ensureFlutterViewEmbedderInitialized();

    // The following parameters are hard-coded in Flutter's test embedder. Since
    // we don't have an embedder yet this is the lowest-most layer we can put
    // this stuff in.
    const double devicePixelRatio = 3.0;
    engine.window.debugOverrideDevicePixelRatio(devicePixelRatio);
    engine.window.webOnlyDebugPhysicalSizeOverride =
        const ui.Size(800 * devicePixelRatio, 600 * devicePixelRatio);
    engine.scheduleFrameCallback = () {};
    ui.debugEmulateFlutterTesterEnvironment = true;

    debugFontsScope = configureDebugFontsAssetScope(fakeAssetManager);
    await engine.initializeEngine(assetManager: fakeAssetManager);
  });

  tearDownAll(() async {
    fakeAssetManager.popAssetScope(debugFontsScope);
  });
}

Future<void>? _platformInitializedFuture;

Future<void> initializeTestFlutterViewEmbedder({double devicePixelRatio = 3.0}) {
  // Force-initialize FlutterViewEmbedder so it doesn't overwrite test pixel ratio.
  engine.ensureFlutterViewEmbedderInitialized();

  // The following parameters are hard-coded in Flutter's test embedder. Since
  // we don't have an embedder yet this is the lowest-most layer we can put
  // this stuff in.
  engine.window.debugOverrideDevicePixelRatio(devicePixelRatio);
  engine.window.webOnlyDebugPhysicalSizeOverride =
      ui.Size(800 * devicePixelRatio, 600 * devicePixelRatio);
  engine.scheduleFrameCallback = () {};
  ui.debugEmulateFlutterTesterEnvironment = true;

  // Initialize platform once and reuse across all tests.
  if (_platformInitializedFuture != null) {
    return _platformInitializedFuture!;
  }
  return _platformInitializedFuture =
      engine.initializeEngine(assetManager: fakeAssetManager);
}
