// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';
import 'dart:math';

import 'scenario.dart';

List<int> _to32(int value) {
  final Uint8List temp = Uint8List(4);
  temp.buffer.asByteData().setInt32(0, value, Endian.little);
  return temp;
}

List<int> _to64(num value) {
  final Uint8List temp = Uint8List(15);
  if (value is double) {
    temp.buffer.asByteData().setFloat64(7, value, Endian.little);
  } else if (value is int) {
    temp.buffer.asByteData().setInt64(7, value, Endian.little);
  }
  return temp;
}

/// A simple platform view.
class PlatformViewScenario extends Scenario with _BasePlatformViewScenarioMixin {
  /// Creates the PlatformView scenario.
  ///
  /// The [window] parameter must not be null.
  PlatformViewScenario(Window window, String text, {int id = 0})
      : assert(window != null),
        super(window) {
    constructScenario(window, text, id);
  }

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);

    addPlatformViewToSceneBuilder(builder, 0);

    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawCircle(const Offset(50, 50), 50, Paint()..color = const Color(0xFFABCDEF));
    final Picture picture = recorder.endRecording();
    builder.addPicture(const Offset(300, 300), picture);

    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

/// Platform view with clip rect.
class PlatformViewClipRectScenario extends Scenario with _BasePlatformViewScenarioMixin {

    /// Constuct a platform view with clip rect scenario.
  PlatformViewClipRectScenario(Window window, String text, {int id = 0})
      : assert(window != null),
        super(window) {
    constructScenario(window, text, id);
  }

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();
    builder.pushOffset(0, 0);
    builder.pushClipRect(Rect.fromLTRB(50, 50, 300, 300));
    addPlatformViewToSceneBuilder(builder, 1);

    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

/// Platform view with clip rrect.
class PlatformViewClipRRectScenario extends PlatformViewScenario {

  /// Constuct a platform view with clip rrect scenario.
  PlatformViewClipRRectScenario(Window window, String text, {int id = 0}) : super(window, text, id: id);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);
    builder.pushClipRRect(RRect.fromLTRBAndCorners(50, 50, 300, 300, topLeft:Radius.circular(15), topRight:Radius.circular(50), bottomLeft:Radius.circular(50)));
    addPlatformViewToSceneBuilder(builder, 2);
    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

/// Platform view with clip path.
class PlatformViewClipPathScenario extends PlatformViewScenario {

  /// Constuct a platform view with clip rrect scenario.
  PlatformViewClipPathScenario(Window window, String text, {int id = 0}) : super(window, text, id: id);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);
    Path path = Path();
    path.moveTo(200, 0);
    path.lineTo(0, 200);
    path.quadraticBezierTo(80, 400, 100, 300);
    path.lineTo(300, 300);
    path.cubicTo(350, 200, 380, 100, 300, 0);
    path.close();
    builder.pushClipPath(path);

    addPlatformViewToSceneBuilder(builder, 3);
    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

/// Platform view with transform.
class PlatformViewTransformScenario extends PlatformViewScenario {

  /// Constuct a platform view with transform scenario.
  PlatformViewTransformScenario(Window window, String text, {int id = 0}) : super(window, text, id: id);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);
    final Float64List matrix4 = Float64List(16);

    // set identify
    matrix4[0] = 1.0;
    matrix4[5] = 1.0;
    matrix4[10] = 1.0;
    matrix4[15] = 1.0;


    // rotate for 1 degree radians
    const double angle = 1;
    final double cosAngle = cos(angle);
    final double sinAngle = sin(angle);
    final double r1 = matrix4[0] * cosAngle + matrix4[4] * sinAngle;
    final double r2 = matrix4[1] * cosAngle + matrix4[5] * sinAngle;
    final double r3 = matrix4[2] * cosAngle + matrix4[6] * sinAngle;
    final double r4 = matrix4[3] * cosAngle + matrix4[7] * sinAngle;
    final double r5 = matrix4[0] * -sinAngle + matrix4[4] * cosAngle;
    final double r6 = matrix4[1] * -sinAngle + matrix4[5] * cosAngle;
    final double r7 = matrix4[2] * -sinAngle + matrix4[6] * cosAngle;
    final double r8 = matrix4[3] * -sinAngle + matrix4[7] * cosAngle;
    matrix4[0] = r1;
    matrix4[1] = r2;
    matrix4[2] = r3;
    matrix4[3] = r4;
    matrix4[4] = r5;
    matrix4[5] = r6;
    matrix4[6] = r7;
    matrix4[7] = r8;

    // scale both x and y by half.
    const double sx = 0.5;
    const double sy = 0.5;

    matrix4[0] *= sx;
    matrix4[1] *= sx;
    matrix4[2] *= sx;
    matrix4[3] *= sx;
    matrix4[4] *= sy;
    matrix4[5] *= sy;
    matrix4[6] *= sy;
    matrix4[7] *= sy;

    // translate for (300, 300)
    matrix4[12] += 300;
    matrix4[13] += 300;

    builder.pushTransform(matrix4);

    addPlatformViewToSceneBuilder(builder, 4);
    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

/// Platform view with opacity.
class PlatformViewOpacityScenario extends PlatformViewScenario {

  /// Constuct a platform view with transform scenario.
  PlatformViewOpacityScenario(Window window, String text, {int id = 0}) : super(window, text, id: id);

  @override
  void onBeginFrame(Duration duration) {
    final SceneBuilder builder = SceneBuilder();

    builder.pushOffset(0, 0);
    builder.pushOpacity(150);

    addPlatformViewToSceneBuilder(builder, 5);
    final Scene scene = builder.build();
    window.render(scene);
    scene.dispose();
  }
}

mixin _BasePlatformViewScenarioMixin on Scenario {

  int _textureId;

  /// Construct the platform view related scenario
  ///
  /// It prepare a TextPlatformView so it can be added to the SceneBuilder in `onBeginFrame`.
  /// Call this method in the constructor of the platform view related scenarios
  /// to perform necessary set up.
  void constructScenario(Window window, String text, int id) {
        const int _valueInt32 = 3;
    const int _valueFloat64 = 6;
    const int _valueString = 7;
    const int _valueUint8List = 8;
    const int _valueMap = 13;
    final Uint8List message = Uint8List.fromList(<int>[
      _valueString,
      'create'.length, // this is safe as long as these are all single byte characters.
      ...utf8.encode('create'),
      _valueMap,
      if (Platform.isIOS)
        3, // 3 entries in map for iOS.
      if (Platform.isAndroid)
        6, // 6 entries in map for Android.
      _valueString,
      'id'.length,
      ...utf8.encode('id'),
      _valueInt32,
      ..._to32(id),
      _valueString,
      'viewType'.length,
      ...utf8.encode('viewType'),
      _valueString,
      'scenarios/textPlatformView'.length,
      ...utf8.encode('scenarios/textPlatformView'),
      if (Platform.isAndroid) ...<int>[
        _valueString,
        'width'.length,
        ...utf8.encode('width'),
        _valueFloat64,
        ..._to64(500.0),
        _valueString,
        'height'.length,
        ...utf8.encode('height'),
        _valueFloat64,
        ..._to64(500.0),
        _valueString,
        'direction'.length,
        ...utf8.encode('direction'),
        _valueInt32,
        ..._to32(0), // LTR
      ],
      _valueString,
      'params'.length,
      ...utf8.encode('params'),
      _valueUint8List,
      text.length,
      ...utf8.encode(text),
    ]);

    window.sendPlatformMessage(
      'flutter/platform_views',
      message.buffer.asByteData(),
      (ByteData response) {
        if (Platform.isAndroid) {
          _textureId = response.getInt64(2);
        }
      },
    );
  }

  // Add a platform view to the `sceneBuilder`.
  void addPlatformViewToSceneBuilder(SceneBuilder sceneBuilder, int viewId) {
    if (Platform.isIOS) {
      sceneBuilder.addPlatformView(viewId, width: 500, height: 500);
    } else if (Platform.isAndroid && _textureId != null) {
      sceneBuilder.addTexture(_textureId, offset: const Offset(150, 300), width: 500, height: 500);
    } else {
      throw UnsupportedError('Platform ${Platform.operatingSystem} is not supported');
    }
  }
}