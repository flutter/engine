// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:js' as js;

import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

export '../common.dart';

/// Used in tests instead of [ProductionCollector] to control Skia object
/// collection explicitly, and to prevent leaks across tests.
///
/// See [TestCollector] for usage.
late TestCollector testCollector;

const MethodCodec codec = StandardMethodCodec();

/// Common test setup for all CanvasKit unit-tests.
void setUpCanvasKitTest() {
  setUpAll(() async {
    expect(useCanvasKit, true, reason: 'This test must run in CanvasKit mode.');
    debugResetBrowserSupportsFinalizationRegistry();
    await ui.webOnlyInitializePlatform(assetManager: WebOnlyMockAssetManager());
  });

  setUp(() async {
    testCollector = TestCollector();
    Collector.debugOverrideCollector(testCollector);
  });

  tearDown(() {
    testCollector.cleanUpAfterTest();
    debugResetBrowserSupportsFinalizationRegistry();
    HtmlViewEmbedder.instance.debugClear();
    SurfaceFactory.instance.debugClear();
  });

  tearDownAll(() {
    debugResetBrowserSupportsFinalizationRegistry();
  });
}

/// Utility function for CanvasKit tests to draw pictures without
/// the [CkPictureRecorder] boilerplate.
CkPicture paintPicture(
    ui.Rect cullRect, void Function(CkCanvas canvas) painter) {
  final CkPictureRecorder recorder = CkPictureRecorder();
  final CkCanvas canvas = recorder.beginRecording(cullRect);
  painter(canvas);
  return recorder.endRecording();
}

class _TestFinalizerRegistration {
  _TestFinalizerRegistration(this.wrapper, this.deletable, this.stackTrace);

  final Object wrapper;
  final SkDeletable deletable;
  final StackTrace stackTrace;
}

class _TestCollection {
  _TestCollection(this.deletable, this.stackTrace);

  final SkDeletable deletable;
  final StackTrace stackTrace;
}

/// Provides explicit synchronous API for collecting Skia objects in tests.
///
/// [ProductionCollector] relies on `FinalizationRegistry` and timers to
/// delete Skia objects, which makes it more precise and efficient. However,
/// it also makes it unpredictable. For example, an object created in one
/// test may be collected while running another test because the timing is
/// subject to browser-specific GC scheduling.
///
/// Tests should use [collectNow] and [collectAfterTest] to trigger collections.
class TestCollector implements Collector {
  final List<_TestFinalizerRegistration> _activeRegistrations =
      <_TestFinalizerRegistration>[];
  final List<_TestFinalizerRegistration> _collectedRegistrations =
      <_TestFinalizerRegistration>[];

  final List<_TestCollection> _pendingCollections = <_TestCollection>[];
  final List<_TestCollection> _completedCollections = <_TestCollection>[];

  @override
  void register(Object wrapper, SkDeletable deletable) {
    _activeRegistrations.add(
      _TestFinalizerRegistration(wrapper, deletable, StackTrace.current),
    );
  }

  @override
  void collect(SkDeletable deletable) {
    _pendingCollections.add(
      _TestCollection(deletable, StackTrace.current),
    );
  }

  /// Deletes all Skia objects scheduled for collection.
  void collectNow() {
    for (final _TestCollection collection in _pendingCollections) {
      late final _TestFinalizerRegistration? activeRegistration;
      for (final _TestFinalizerRegistration registration in _activeRegistrations) {
        if (identical(registration.deletable, collection.deletable)) {
          activeRegistration = registration;
          break;
        }
      }
      if (activeRegistration == null) {
        late final _TestFinalizerRegistration? collectedRegistration;
        for (final _TestFinalizerRegistration registration
            in _collectedRegistrations) {
          if (identical(registration.deletable, collection.deletable)) {
            collectedRegistration = registration;
            break;
          }
        }
        if (collectedRegistration == null) {
          fail(
              'Attempted to collect an object that was never registered for finalization.\n'
              'The collection was requested here:\n'
              '${collection.stackTrace}');
        } else {
          final _TestCollection firstCollection = _completedCollections
              .firstWhere((_TestCollection completedCollection) {
            return identical(
                completedCollection.deletable, collection.deletable);
          });
          fail(
            'Attempted to collect an object that was previously collected.\n'
            'The object was registered for finalization here:\n'
            '${collection.stackTrace}\n\n'
            'The first collection was requested here:\n'
            '${firstCollection.stackTrace}\n\n'
            'The second collection was requested here:\n'
            '${collection.stackTrace}',
          );
        }
      } else {
        _collectedRegistrations.add(activeRegistration);
        _activeRegistrations.remove(activeRegistration);
        _completedCollections.add(collection);
        if (!collection.deletable.isDeleted()) {
          collection.deletable.delete();
        }
      }
    }
    _pendingCollections.clear();
  }

  /// Deletes all Skia objects with registered finalizers.
  ///
  /// This also deletes active objects that have not been scheduled for
  /// collection, to prevent objects leaking across tests.
  void cleanUpAfterTest() {
    for (final _TestCollection collection in _pendingCollections) {
      if (!collection.deletable.isDeleted()) {
        collection.deletable.delete();
      }
    }
    for (final _TestFinalizerRegistration registration in _activeRegistrations) {
      if (!registration.deletable.isDeleted()) {
        registration.deletable.delete();
      }
    }
    _activeRegistrations.clear();
    _collectedRegistrations.clear();
    _pendingCollections.clear();
    _completedCollections.clear();
  }
}

/// Checks that a [picture] matches the [goldenFile].
///
/// The picture is drawn onto the UI at [ui.Offset.zero] with no additional
/// layers.
Future<void> matchPictureGolden(String goldenFile, CkPicture picture,
    {required ui.Rect region, bool write = false}) async {
  final EnginePlatformDispatcher dispatcher =
      ui.window.platformDispatcher as EnginePlatformDispatcher;
  final LayerSceneBuilder sb = LayerSceneBuilder();
  sb.pushOffset(0, 0);
  sb.addPicture(ui.Offset.zero, picture);
  dispatcher.rasterizer!.draw(sb.build().layerTree);
  await matchGoldenFile(goldenFile,
      region: region, maxDiffRatePercent: 0.0, write: write);
}

/// Sends a platform message to create a Platform View with the given id and viewType.
Future<void> createPlatformView(int id, String viewType) {
  final Completer<void> completer = Completer<void>();
  window.sendPlatformMessage(
    'flutter/platform_views',
    codec.encodeMethodCall(MethodCall(
      'create',
      <String, dynamic>{
        'id': id,
        'viewType': viewType,
      },
    )),
    (dynamic _) => completer.complete(),
  );
  return completer.future;
}

/// Disposes of the platform view with the given [id].
Future<void> disposePlatformView(int id) {
  final Completer<void> completer = Completer<void>();
  window.sendPlatformMessage(
    'flutter/platform_views',
    codec.encodeMethodCall(MethodCall('dispose', id)),
    (dynamic _) => completer.complete(),
  );
  return completer.future;
}

/// Creates a pre-laid out one-line paragraph of text.
///
/// Useful in tests that need a simple label to annotate goldens.
CkParagraph makeSimpleText(String text, {
  String? fontFamily,
  double? fontSize,
  ui.FontStyle? fontStyle,
  ui.FontWeight? fontWeight,
  ui.Color? color,
}) {
  final CkParagraphBuilder builder = CkParagraphBuilder(CkParagraphStyle(
    fontFamily: fontFamily ?? 'Roboto',
    fontSize: fontSize ?? 14,
    fontStyle: fontStyle ?? ui.FontStyle.normal,
    fontWeight: fontWeight ?? ui.FontWeight.normal,
  ));
  builder.pushStyle(CkTextStyle(
    color: color ?? const ui.Color(0xFF000000),
  ));
  builder.addText(text);
  builder.pop();
  final CkParagraph paragraph = builder.build();
  paragraph.layout(const ui.ParagraphConstraints(width: 10000));
  return paragraph;
}

class _PatchedH5vcc implements H5vcc {
  @override
  final CanvasKit canvasKit;

  _PatchedH5vcc(this.canvasKit);
}

/// Test setup to initialize `window.h5vcc` with a patched H5vcc implementation.
///
/// The patched H5vcc implementation uses a downloaded CanvasKit implementation
/// monkey-patched with a fake getH5vccSkSurface function.
///
/// This function should be called before initialization, i.e. before
/// [setUpCanvasKitTest].
void patchH5vccCanvasKit({Function? onGetH5vccSkSurfaceCalled}) {
  CanvasKit? existingCanvasKit;
  SkiaFontCollection? existingSkiaFontCollection;

  setUpAll(() async {
    // Clear out any existing windowFlutterCanvasKit. This ensures that in the
    // call to [initializeCanvasKit], no cached value is used.
    existingCanvasKit = windowFlutterCanvasKit;
    windowFlutterCanvasKit = null;

    // Clear out any existing skiaFontCollection. If a SkiaFontCollection was
    // previously initialized with a non-h5vcc-patched CanvasKit, it is
    // incompatible with the h5vcc-patched CanvasKit we create in this function.
    // The next call to [ensureSkiaFontCollectionInitialized] will create a new
    // SkiaFontCollection.
    existingSkiaFontCollection = skiaFontCollection;
    debugSetSkiaFontCollection(null);

    // Set `window.h5vcc` to _PatchedH5vcc which uses a downloaded CanvasKit.
    final CanvasKit downloadedCanvasKit = await downloadCanvasKit();
    debugH5vccSetter = _PatchedH5vcc(downloadedCanvasKit);

    // Monkey-patch the getH5vccSkSurface function of `window.h5vcc.canvasKit`.
    js.context['h5vcc']['canvasKit']['getH5vccSkSurface'] = () {
      if (onGetH5vccSkSurfaceCalled != null) {
        onGetH5vccSkSurfaceCalled();
      }

      // Returns a fake [SkSurface] object with a minimal implementation.
      return js.JsObject.jsify(<String, dynamic>{
        'dispose': () {}
      });
    };
  });

  tearDownAll(() {
    // Unset `window.h5vcc`.
    debugH5vccSetter = null;

    // Reset original windowFlutterCanvasKit and skiaFontCollection values.
    windowFlutterCanvasKit = existingCanvasKit;
    debugSetSkiaFontCollection(existingSkiaFontCollection);
  });
}
