// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data' show Float64List;
import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('pushTransform validates the matrix', () {
    final SceneBuilder builder = SceneBuilder();
    final Float64List matrix4 = Float64List.fromList(<double>[
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1,
    ]);
    expect(builder.pushTransform(matrix4), isNotNull);

    final Float64List matrix4WrongLength = Float64List.fromList(<double>[
      1, 0, 0, 0,
      0, 1, 0,
      0, 0, 1, 0,
      0, 0, 0,
    ]);
    expect(
      () => builder.pushTransform(matrix4WrongLength),
      throwsA(const TypeMatcher<AssertionError>()),
    );

    final Float64List matrix4NaN = Float64List.fromList(<double>[
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, double.nan,
    ]);
    expect(
      () => builder.pushTransform(matrix4NaN),
      throwsA(const TypeMatcher<AssertionError>()),
    );

    final Float64List matrix4Infinity = Float64List.fromList(<double>[
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, double.infinity,
    ]);
    expect(
      () => builder.pushTransform(matrix4Infinity),
      throwsA(const TypeMatcher<AssertionError>()),
    );
  });

  test('SceneBuilder accepts typed layers', () {
    final SceneBuilder builder1 = SceneBuilder();
    final OpacityEngineLayer opacity1 = builder1.pushOpacity(100);
    expect(opacity1, isNotNull);
    builder1.pop();
    builder1.build();

    final SceneBuilder builder2 = SceneBuilder();
    final OpacityEngineLayer opacity2 = builder2.pushOpacity(200, oldLayer: opacity1);
    expect(opacity2, isNotNull);
    builder2.pop();
    builder2.build();
  });

  void testNoSharing(_TestNoSharingFunction pushFunction) {
    final SceneBuilder builder1 = SceneBuilder();
    final EngineLayer layer = pushFunction(builder1, null);
    final EngineLayer childLayer = builder1.pushOpacity(123);
    builder1.pop();
    builder1.pop();
    builder1.build();

    // Test: first push then attempt illegal addRetained
    final SceneBuilder builder2 = SceneBuilder();
    pushFunction(builder2, layer);
    builder2.pop();
    try {
      builder2.addRetained(layer);
      fail('Expected addRetained to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder2.build();

    // Test: first addRetained then attempt illegal push
    final SceneBuilder builder3 = SceneBuilder();
    builder3.addRetained(layer);
    try {
      pushFunction(builder3, layer);
      fail('Expected push to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder3.build();

    // Test: addRetained twice
    final SceneBuilder builder4 = SceneBuilder();
    builder4.addRetained(layer);
    try {
      builder4.addRetained(layer);
      fail('Expected second addRetained to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder4.build();

    // Test: push twice
    final SceneBuilder builder5 = SceneBuilder();
    pushFunction(builder5, layer);
    try {
      pushFunction(builder5, layer);
      fail('Expected push to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder5.build();

    // Test: child layer of a retained layer also pushed
    final SceneBuilder builder6 = SceneBuilder();
    builder6.addRetained(layer);
    try {
      builder6.pushOpacity(321, oldLayer: childLayer);
      fail('Expected pushOpacity to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder6.build();

    // Test: pushed layer's parent being also added as retained
    final SceneBuilder builder7 = SceneBuilder();
    builder7.pushOpacity(234, oldLayer: childLayer);
    builder7.pop();
    try {
      builder7.addRetained(layer);
      fail('Expected addRetained to throw AssertionError but it returned successully');
    } on AssertionError catch (error) {
      expect(error.toString(), contains('The layer is already being used'));
    }
    builder7.build();
  }

  test('SceneBuilder does not share a layer between addRetained and push*', () {
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushOffset(0, 0, oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushTransform(Float64List(16), oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushClipRect(Rect.zero, oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushClipRRect(RRect.zero, oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushClipPath(Path(), oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushOpacity(100, oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushColorFilter(const Color.fromARGB(0, 0, 0, 0), BlendMode.color, oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushBackdropFilter(ImageFilter.blur(), oldLayer: oldLayer);
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushShaderMask(
        Gradient.radial(
          const Offset(0, 0),
          10,
          const <Color>[Color.fromARGB(0, 0, 0, 0), Color.fromARGB(0, 255, 255, 255)],
        ),
        Rect.zero,
        BlendMode.color,
        oldLayer: oldLayer,
      );
    });
    testNoSharing((SceneBuilder builder, EngineLayer oldLayer) {
      return builder.pushPhysicalShape(path: Path(), color: const Color.fromARGB(0, 0, 0, 0), oldLayer: oldLayer);
    });
  });
}

typedef _TestNoSharingFunction = EngineLayer Function(SceneBuilder builder, EngineLayer oldLayer);
