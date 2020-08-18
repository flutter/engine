// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10
part of engine;

/// Whether the CanvasKit renderer should always be used.
// TODO(yjbanov): rename to "FLUTTER_WEB_USE_CANVASKIT_RENDERER"
const bool forceCanvasKitRenderer =
    bool.fromEnvironment('FLUTTER_WEB_USE_SKIA', defaultValue: false);

/// Whether the HTML renderer should always be used.
const bool forceHtmlRenderer =
    bool.fromEnvironment('FLUTTER_WEB_FORCE_HTML_RENDERER', defaultValue: false);

/// Whether the CanvasKit renderer should be used as detected dynamically based
/// on the current browser/OS combination.
bool? _shouldUseCanvasKit;

/// Whether the CanvasKit renderer should be used.
@pragma('dart2js:tryInline')
bool get useCanvasKit => forceCanvasKitRenderer || !forceHtmlRenderer && (_shouldUseCanvasKit ??= detectShouldUseCanvasKit());

/// Detects whether the CanvasKit renderer should be used.
///
/// We prefer CanvasKit on desktop browsers that have sufficient support for
/// WebGL and WebAssembly. Otherwise, we prefer the HTML renderer (e.g. IE11,
/// mobile browsers). Preference for HTML renderer on mobile browsers is due to
/// bandwidth constraints, as CanvasKit requires an extra 2MB in download.
@pragma('dart2js:noInline')
bool detectShouldUseCanvasKit() {
  // This is mostly to filter our IE11 and old Edge, which do not have the
  // necessary features to run CanvasKit.
  final bool isSupportedBrowserEngine =
      browserEngine == BrowserEngine.blink ||
      browserEngine == BrowserEngine.firefox ||
      browserEngine == BrowserEngine.webkit ||
      browserEngine == BrowserEngine.unknown;

  if (!isSupportedBrowserEngine) {
    return false;
  }

  // Mobile operating system, although capable of running CanvasKit, tend to be
  // used on-the-go, with spotty Internet connections. Because as of this writing
  // CanvasKit requires an extra 2MB of download size, the "auto" mode prefers
  // the HTML renderer.
  final bool isMobile =
      operatingSystem == OperatingSystem.android ||
      operatingSystem == OperatingSystem.iOs;
  return !isMobile;
}

// If set to true, forces CPU-only rendering (i.e. no WebGL).
const bool canvasKitForceCpuOnly =
    bool.fromEnvironment('FLUTTER_WEB_CANVASKIT_FORCE_CPU_ONLY', defaultValue: false);

/// The URL to use when downloading the CanvasKit script and associated wasm.
///
/// When CanvasKit pushes a new release to NPM, update this URL to reflect the
/// most recent version. For example, if CanvasKit releases version 0.34.0 to
/// NPM, update this URL to `https://unpkg.com/canvaskit-wasm@0.34.0/bin/`.
const String canvasKitBaseUrl = String.fromEnvironment(
  'FLUTTER_WEB_CANVASKIT_URL',
  defaultValue: 'https://unpkg.com/canvaskit-wasm@0.17.3/bin/',
);

/// Initialize CanvasKit.
///
/// This calls `CanvasKitInit` and assigns the global [canvasKit] object.
Future<void> initializeCanvasKit() {
  final Completer<void> canvasKitCompleter = Completer<void>();
  late StreamSubscription<html.Event> loadSubscription;
  loadSubscription = domRenderer.canvasKitScript!.onLoad.listen((_) {
    loadSubscription.cancel();
    final CanvasKitInitPromise canvasKitInitPromise = CanvasKitInit(CanvasKitInitOptions(
      locateFile: js.allowInterop((String file, String unusedBase) => canvasKitBaseUrl + file),
    ));
    canvasKitInitPromise.then(js.allowInterop((CanvasKit ck) {
      canvasKit = ck;
      windowFlutterCanvasKit = canvasKit;
      canvasKitCompleter.complete();
    }));
  });

  /// Add a Skia scene host.
  skiaSceneHost = html.Element.tag('flt-scene');
  domRenderer.renderScene(skiaSceneHost);
  return canvasKitCompleter.future;
}

/// The Skia font collection.
SkiaFontCollection get skiaFontCollection => _skiaFontCollection!;
SkiaFontCollection? _skiaFontCollection;

/// Initializes [skiaFontCollection].
void ensureSkiaFontCollectionInitialized() {
  _skiaFontCollection ??= SkiaFontCollection();
}

/// The scene host, where the root canvas and overlay canvases are added to.
html.Element? skiaSceneHost;
