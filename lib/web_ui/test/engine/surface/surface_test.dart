// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;

import '../../common/test_initialization.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('Surface', () {
    setUpAll(() async {
      await bootstrapAndRunApp(withImplicitView: true);
    });

    setUp(() {
      SurfaceSceneBuilder.debugForgetFrameScene();
    });

    test('debugAssertSurfaceState produces a human-readable message', () {
      final builder = SceneBuilder();
      final opacityLayer = builder.pushOpacity(100) as PersistedOpacity;
      try {
        debugAssertSurfaceState(opacityLayer, PersistedSurfaceState.active, PersistedSurfaceState.pendingRetention);
        fail('Expected $PersistedSurfaceException');
      } on PersistedSurfaceException catch (exception) {
        expect(
          '$exception',
          'PersistedOpacity: is in an unexpected state.\n'
          'Expected one of: PersistedSurfaceState.active, PersistedSurfaceState.pendingRetention\n'
          'But was: PersistedSurfaceState.created',
        );
      }
    });

    test('is created', () {
      final builder = SceneBuilder();
      final opacityLayer = builder.pushOpacity(100) as PersistedOpacity;
      builder.pop();

      expect(opacityLayer, isNotNull);
      expect(opacityLayer.rootElement, isNull);
      expect(opacityLayer.isCreated, isTrue);

      builder.build();

      expect(opacityLayer.rootElement!.tagName.toLowerCase(), 'flt-opacity');
      expect(opacityLayer.isActive, isTrue);
    });

    test('is released', () {
      final builder1 = SceneBuilder();
      final opacityLayer = builder1.pushOpacity(100) as PersistedOpacity;
      builder1.pop();
      builder1.build();
      expect(opacityLayer.isActive, isTrue);

      SceneBuilder().build();
      expect(opacityLayer.isReleased, isTrue);
      expect(opacityLayer.rootElement, isNull);
    });

    test('discarding is recursive', () {
      final builder1 = SceneBuilder();
      final opacityLayer = builder1.pushOpacity(100) as PersistedOpacity;
      final transformLayer =
          builder1.pushTransform(Matrix4.identity().toFloat64()) as PersistedTransform;
      builder1.pop();
      builder1.pop();
      builder1.build();
      expect(opacityLayer.isActive, isTrue);
      expect(transformLayer.isActive, isTrue);

      SceneBuilder().build();
      expect(opacityLayer.isReleased, isTrue);
      expect(transformLayer.isReleased, isTrue);
      expect(opacityLayer.rootElement, isNull);
      expect(transformLayer.rootElement, isNull);
    });

    test('is updated', () {
      final builder1 = SceneBuilder();
      final opacityLayer1 = builder1.pushOpacity(100) as PersistedOpacity;
      builder1.pop();
      builder1.build();
      expect(opacityLayer1.isActive, isTrue);
      final element = opacityLayer1.rootElement!;

      final builder2 = SceneBuilder();
      final opacityLayer2 =
          builder2.pushOpacity(200, oldLayer: opacityLayer1) as PersistedOpacity;
      expect(opacityLayer1.isPendingUpdate, isTrue);
      expect(opacityLayer2.isCreated, isTrue);
      expect(opacityLayer2.oldLayer, same(opacityLayer1));
      builder2.pop();

      builder2.build();
      expect(opacityLayer1.isReleased, isTrue);
      expect(opacityLayer1.rootElement, isNull);
      expect(opacityLayer2.isActive, isTrue);
      expect(
          opacityLayer2.rootElement, element); // adopts old surface's element
      expect(opacityLayer2.oldLayer, isNull);
    });

    test('ignores released surface when updated', () {
      // Build a surface
      final builder1 = SceneBuilder();
      final opacityLayer1 = builder1.pushOpacity(100) as PersistedOpacity;
      builder1.pop();
      builder1.build();
      expect(opacityLayer1.isActive, isTrue);
      final element = opacityLayer1.rootElement!;

      // Release it
      SceneBuilder().build();
      expect(opacityLayer1.isReleased, isTrue);
      expect(opacityLayer1.rootElement, isNull);

      // Attempt to update it
      final builder2 = SceneBuilder();
      final opacityLayer2 =
          builder2.pushOpacity(200, oldLayer: opacityLayer1) as PersistedOpacity;
      builder2.pop();
      expect(opacityLayer1.isReleased, isTrue);
      expect(opacityLayer2.isCreated, isTrue);

      builder2.build();
      expect(opacityLayer1.isReleased, isTrue);
      expect(opacityLayer2.isActive, isTrue);
      expect(opacityLayer2.rootElement, isNot(equals(element)));
    });

    // This test creates a situation when an intermediate layer disappears,
    // causing its child to become a direct child of the common ancestor. This
    // often happens with opacity layers. When opacity reaches 1.0, the
    // framework removes that layer (as it is no longer necessary). This test
    // makes sure we reuse the child layer's DOM nodes. Here's the illustration
    // of what's happening:
    //
    // Frame 1   Frame 2
    //
    //   A         A
    //   |         |
    //   B     ┌──>C
    //   |     │   |
    //   C ────┘   L
    //   |
    //   L
    //
    // Layer "L" is a logging layer used to track what would happen to the
    // child of "C" as it's being dragged around the tree. For example, we
    // check that the child doesn't get discarded by mistake.
    test('reparents DOM element when updated', () {
      final logger = _LoggingTestSurface();
      final builder1 = SurfaceSceneBuilder();
      final a1 =
          builder1.pushTransform(
              (Matrix4.identity()..scale(EngineFlutterDisplay.instance.browserDevicePixelRatio)).toFloat64()) as PersistedTransform;
      final b1 = builder1.pushOpacity(100) as PersistedOpacity;
      final c1 =
          builder1.pushTransform(Matrix4.identity().toFloat64()) as PersistedTransform;
      builder1.debugAddSurface(logger);
      builder1.pop();
      builder1.pop();
      builder1.pop();
      builder1.build();
      expect(logger.log, <String>['build', 'createElement', 'apply']);

      final elementA = a1.rootElement!;
      final elementB = b1.rootElement!;
      final elementC = c1.rootElement!;

      expect(elementC.parent, elementB);
      expect(elementB.parent, elementA);

      final builder2 = SurfaceSceneBuilder();
      final a2 =
          builder2.pushTransform(
              (Matrix4.identity()..scale(EngineFlutterDisplay.instance.browserDevicePixelRatio)).toFloat64(),
              oldLayer: a1) as PersistedTransform;
      final c2 =
          builder2.pushTransform(Matrix4.identity().toFloat64(), oldLayer: c1) as PersistedTransform;
      builder2.addRetained(logger);
      builder2.pop();
      builder2.pop();

      expect(c1.isPendingUpdate, isTrue);
      expect(c2.isCreated, isTrue);
      builder2.build();
      expect(logger.log, <String>['build', 'createElement', 'apply', 'retain']);
      expect(c1.isReleased, isTrue);
      expect(c2.isActive, isTrue);

      expect(a2.rootElement, elementA);
      expect(b1.rootElement, isNull);
      expect(c2.rootElement, elementC);

      expect(elementC.parent, elementA);
      expect(elementB.parent, null);
    },
        // This method failed on iOS Safari.
        // TODO(ferhat): https://github.com/flutter/flutter/issues/60036
        skip: ui_web.browser.browserEngine == ui_web.BrowserEngine.webkit &&
            ui_web.browser.operatingSystem == ui_web.OperatingSystem.iOs);

    test('is retained', () {
      final builder1 = SceneBuilder();
      final opacityLayer = builder1.pushOpacity(100) as PersistedOpacity;
      builder1.pop();
      builder1.build();
      expect(opacityLayer.isActive, isTrue);
      final element = opacityLayer.rootElement!;

      final builder2 = SceneBuilder();

      expect(opacityLayer.isActive, isTrue);
      builder2.addRetained(opacityLayer);
      expect(opacityLayer.isPendingRetention, isTrue);

      builder2.build();
      expect(opacityLayer.isActive, isTrue);
      expect(opacityLayer.rootElement, element);
    });

    test('revives released surface when retained', () {
      final builder1 = SurfaceSceneBuilder();
      final opacityLayer = builder1.pushOpacity(100) as PersistedOpacity;
      final logger = _LoggingTestSurface();
      builder1.debugAddSurface(logger);
      builder1.pop();
      builder1.build();
      expect(opacityLayer.isActive, isTrue);
      expect(logger.log, <String>['build', 'createElement', 'apply']);
      final element = opacityLayer.rootElement!;

      SceneBuilder().build();
      expect(opacityLayer.isReleased, isTrue);
      expect(opacityLayer.rootElement, isNull);
      expect(logger.log, <String>['build', 'createElement', 'apply', 'discard']);

      final builder2 = SceneBuilder();
      builder2.addRetained(opacityLayer);
      expect(opacityLayer.isCreated, isTrue); // revived
      expect(logger.log, <String>['build', 'createElement', 'apply', 'discard', 'revive']);

      builder2.build();
      expect(opacityLayer.isActive, isTrue);
      expect(opacityLayer.rootElement, isNot(equals(element)));
    });

    test('reviving is recursive', () {
      final builder1 = SceneBuilder();
      final opacityLayer = builder1.pushOpacity(100) as PersistedOpacity;
      final transformLayer =
          builder1.pushTransform(Matrix4.identity().toFloat64()) as PersistedTransform;
      builder1.pop();
      builder1.pop();
      builder1.build();
      expect(opacityLayer.isActive, isTrue);
      expect(transformLayer.isActive, isTrue);
      final opacityElement = opacityLayer.rootElement!;
      final transformElement = transformLayer.rootElement!;

      SceneBuilder().build();

      final builder2 = SceneBuilder();
      builder2.addRetained(opacityLayer);
      expect(opacityLayer.isCreated, isTrue); // revived
      expect(transformLayer.isCreated, isTrue); // revived

      builder2.build();
      expect(opacityLayer.isActive, isTrue);
      expect(transformLayer.isActive, isTrue);
      expect(opacityLayer.rootElement, isNot(equals(opacityElement)));
      expect(transformLayer.rootElement, isNot(equals(transformElement)));
    });

    // This test creates a situation when a retained layer is moved to another
    // parent. We want to make sure that we move the retained layer's elements
    // without rebuilding from scratch. No new elements are created in this
    // situation.
    //
    // Here's an illustrated example where layer C is reparented onto B along
    // with D:
    //
    // Frame 1   Frame 2
    //
    //    A         A
    //   ╱ ╲        |
    //  B   C ──┐   B
    //      |   │   |
    //      D   └──>C
    //              |
    //              D
    test('reparents DOM elements when retained', () {
      final builder1 = SceneBuilder();
      final a1 = builder1.pushOpacity(10) as PersistedOpacity;
      final b1 = builder1.pushOpacity(20) as PersistedOpacity;
      builder1.pop();
      final c1 = builder1.pushOpacity(30) as PersistedOpacity;
      final d1 = builder1.pushOpacity(40) as PersistedOpacity;
      builder1.pop();
      builder1.pop();
      builder1.pop();
      builder1.build();

      final elementA = a1.rootElement!;
      final elementB = b1.rootElement!;
      final elementC = c1.rootElement!;
      final elementD = d1.rootElement!;

      expect(elementB.parent, elementA);
      expect(elementC.parent, elementA);
      expect(elementD.parent, elementC);

      final builder2 = SceneBuilder();
      final a2 = builder2.pushOpacity(10, oldLayer: a1) as PersistedOpacity;
      final b2 = builder2.pushOpacity(20, oldLayer: b1) as PersistedOpacity;
      builder2.addRetained(c1);
      builder2.pop();
      builder2.pop();
      builder2.build();

      expect(a2.rootElement, elementA);
      expect(b2.rootElement, elementB);
      expect(c1.rootElement, elementC);
      expect(d1.rootElement, elementD);

      expect(
        <DomElement>[
          elementD.parent!,
          elementC.parent!,
          elementB.parent!,
        ],
        <DomElement>[elementC, elementB, elementA],
      );
    });

    test('is updated by matching', () {
      final builder1 = SceneBuilder();
      final opacityLayer1 = builder1.pushOpacity(100) as PersistedOpacity;
      builder1.pop();
      builder1.build();
      expect(opacityLayer1.isActive, isTrue);
      final element = opacityLayer1.rootElement!;

      final builder2 = SceneBuilder();
      final opacityLayer2 = builder2.pushOpacity(200) as PersistedOpacity;
      expect(opacityLayer1.isActive, isTrue);
      expect(opacityLayer2.isCreated, isTrue);
      builder2.pop();

      builder2.build();
      expect(opacityLayer1.isReleased, isTrue);
      expect(opacityLayer1.rootElement, isNull);
      expect(opacityLayer2.isActive, isTrue);
      expect(
          opacityLayer2.rootElement, element); // adopts old surface's element
    });
  });

  final layerFactories = <String, TestEngineLayerFactory>{
    'ColorFilterEngineLayer': (SurfaceSceneBuilder builder) => builder.pushColorFilter(const ColorFilter.mode(
      Color(0xFFFF0000),
      BlendMode.srcIn,
    )),
    'OffsetEngineLayer': (SurfaceSceneBuilder builder) => builder.pushOffset(1, 2),
    'TransformEngineLayer': (SurfaceSceneBuilder builder) => builder.pushTransform(Matrix4.identity().toFloat64()),
    'ClipRectEngineLayer': (SurfaceSceneBuilder builder) => builder.pushClipRect(const Rect.fromLTRB(0, 0, 10, 10)),
    'ClipRRectEngineLayer': (SurfaceSceneBuilder builder) => builder.pushClipRRect(RRect.fromRectXY(const Rect.fromLTRB(0, 0, 10, 10), 1, 2)),
    'ClipPathEngineLayer': (SurfaceSceneBuilder builder) => builder.pushClipPath(Path()..addRect(const Rect.fromLTRB(0, 0, 10, 10))),
    'OpacityEngineLayer': (SurfaceSceneBuilder builder) => builder.pushOpacity(100),
    'ImageFilterEngineLayer': (SurfaceSceneBuilder builder) => builder.pushImageFilter(ImageFilter.blur(sigmaX: 0.1, sigmaY: 0.2)),
    'BackdropEngineLayer': (SurfaceSceneBuilder builder) => builder.pushBackdropFilter(ImageFilter.blur(sigmaX: 0.1, sigmaY: 0.2)),
    // Firefox does not support WebGL in headless mode.
    if (!isFirefox)
      'ShaderMaskEngineLayer': (SurfaceSceneBuilder builder) {
        const colors = <Color>[Color(0xFF000000), Color(0xFFFF3C38)];
        const stops = <double>[0.0, 1.0];
        const shaderBounds = Rect.fromLTWH(180, 10, 140, 140);
        final EngineGradient shader = GradientLinear(
          Offset(200 - shaderBounds.left, 30 - shaderBounds.top),
          Offset(320 - shaderBounds.left, 150 - shaderBounds.top),
          colors, stops, TileMode.clamp, Matrix4.identity().storage,
        );
        return builder.pushShaderMask(shader, shaderBounds, BlendMode.srcOver);
      },
  };

  // Regression test for https://github.com/flutter/flutter/issues/104305
  for (final layerFactory in layerFactories.entries) {
    test('${layerFactory.key} supports addRetained after being discarded', () async {
      final builder = SurfaceSceneBuilder();
      builder.pushOffset(0, 0);
      final oldLayer = layerFactory.value(builder) as PersistedSurface;
      builder.pop();
      builder.pop();
      builder.build();
      expect(oldLayer.isActive, isTrue);

      // Pump an empty frame so the `oldLayer` is discarded before it's reused.
      final builder2 = SurfaceSceneBuilder();
      builder2.build();
      expect(oldLayer.isReleased, isTrue);

      // At this point the `oldLayer` needs to be revived.
      final builder3 = SurfaceSceneBuilder();
      builder3.addRetained(oldLayer);
      builder3.build();
      expect(oldLayer.isActive, isTrue);
    });
  }
}

typedef TestEngineLayerFactory = EngineLayer Function(SurfaceSceneBuilder builder);

class _LoggingTestSurface extends PersistedContainerSurface {
  _LoggingTestSurface() : super(null);

  final List<String> log = <String>[];

  @override
  void build() {
    log.add('build');
    super.build();
  }

  @override
  void apply() {
    log.add('apply');
  }

  @override
  DomElement createElement() {
    log.add('createElement');
    return createDomElement('flt-test-layer');
  }

  @override
  void update(_LoggingTestSurface oldSurface) {
    log.add('update');
    super.update(oldSurface);
  }

  @override
  void adoptElements(covariant PersistedSurface oldSurface) {
    log.add('adoptElements');
    super.adoptElements(oldSurface);
  }

  @override
  void retain() {
    log.add('retain');
    super.retain();
  }

  @override
  void discard() {
    log.add('discard');
    super.discard();
  }

  @override
  void revive() {
    log.add('revive');
    super.revive();
  }

  @override
  double matchForUpdate(PersistedSurface? existingSurface) {
    return 1.0;
  }
}
