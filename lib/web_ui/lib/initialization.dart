// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

Future<void> webOnlyInitializePlatform({
  engine.AssetManager? assetManager,
}) async {
  engine.initializeEngine();

  // This needs to be after `initializeEngine` because that is where the
  // canvaskit script is added to the page.
  if (engine.useCanvasKit) {
    await engine.initializeCanvasKit();
  }

  assetManager ??= const engine.AssetManager();
  await _setAssetManager(assetManager);
  if (engine.useCanvasKit) {
    await engine.skiaFontCollection.ensureFontsLoaded();
  } else {
    await _fontCollection!.ensureFontsLoaded();
  }
}

bool get debugEmulateFlutterTesterEnvironment =>
    _debugEmulateFlutterTesterEnvironment;

set debugEmulateFlutterTesterEnvironment(bool value) {
  _debugEmulateFlutterTesterEnvironment = value;
  if (_debugEmulateFlutterTesterEnvironment) {
    const Size logicalSize = Size(800.0, 600.0);
    engine.window.webOnlyDebugPhysicalSizeOverride =
        logicalSize * window.devicePixelRatio;
  }
}

engine.AssetManager get webOnlyAssetManager => _assetManager!;
engine.FontCollection get webOnlyFontCollection => _fontCollection!;

void webOnlySetPluginHandler(Future<void> Function(String, ByteData?, PlatformMessageResponseCallback?) handler) {
  engine.pluginMessageCallHandler = handler;
}

/// A function which takes a unique `id` and creates an HTML element.
typedef PlatformViewFactory = html.Element Function(int viewId);

/// A registry for factories that create platform views.
class PlatformViewRegistry {
  /// Register [viewTypeId] as being creating by the given [factory].
  bool registerViewFactory(String viewTypeId, PlatformViewFactory viewFactory,
      {bool isVisible = true}) {
    // TODO(web): Deprecate this once there's another way of calling `registerFactory` (js interop?)
    return engine.platformViewManager
        .registerFactory(viewTypeId, viewFactory, isVisible: isVisible);
  }
}

/// The platform view registry for this app.
final PlatformViewRegistry platformViewRegistry = PlatformViewRegistry();

///////////////////////////////////////////////////////////////////
/// MOVE EVERYTHING BELOW TO engine.dart
///////////////////////////////////////////////////////////////////

engine.AssetManager? _assetManager;
engine.FontCollection? _fontCollection;

Future<void> _setAssetManager(engine.AssetManager assetManager) async {
  // ignore: unnecessary_null_comparison
  assert(assetManager != null, 'Cannot set assetManager to null');
  if (assetManager == _assetManager) {
    return;
  }

  _assetManager = assetManager;

  if (engine.useCanvasKit) {
    engine.ensureSkiaFontCollectionInitialized();
  } else {
    _fontCollection ??= engine.FontCollection();
    _fontCollection!.clear();
  }

  if (_assetManager != null) {
    if (engine.useCanvasKit) {
      await engine.skiaFontCollection.registerFonts(_assetManager!);
    } else {
      await _fontCollection!.registerFonts(_assetManager!);
    }
  }

  if (debugEmulateFlutterTesterEnvironment) {
    if (engine.useCanvasKit) {
      engine.skiaFontCollection.debugRegisterTestFonts();
    } else {
      _fontCollection!.debugRegisterTestFonts();
    }
  }
}

bool _debugEmulateFlutterTesterEnvironment = false;
