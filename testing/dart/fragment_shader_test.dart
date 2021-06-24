// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:async';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';

import 'package:path/path.dart' as path;
import 'package:litetest/litetest.dart';

void main() {
  test('throws exception for invalid shader', () {
    final ByteBuffer invalidBytes = Uint8List.fromList(<int>[1, 2, 3, 4, 5]).buffer;
    expect(() => FragmentShader(spirv: invalidBytes), throws);
  });

  // TODO(clocksmith): test simple shader.
  // TODO(clocksmith): fix uniform shader test.

  test('renders and udpates shader with uniforms', () async {
    final Uint8List shaderBytes = await File(path.join(
      'flutter',
      'testing',
      'resources',
      'fragment_shader_uniforms.spv',
    )).readAsBytes();
    final FragmentShader shader = FragmentShader(spirv: shaderBytes.buffer);

    shader.update(floatUniforms: Float32List.fromList(<double>[
      0.0,  // iFloatUniform
      0.25, // iVec2Uniform.x
      0.75, // iVec2Uniform.y
      0,    // iMat2Uniform[0][0]
      0,    // iMat2Uniform[0][1]
      0,    // iMat2Uniform[1][0]
      1,    // iMat2Uniform[1][1]
    ]));

    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    final Paint paint = Paint()..shader = shader;
    canvas.drawPaint(paint);
    final Picture picture = recorder.endRecording();
    final Image image = await picture.toImage(100, 100);
    final ByteData renderedBytes = await image.toByteData();

    expect(toFloat(renderedBytes.getUint8(0)), closeTo(0.0, epsilon));
    expect(toFloat(renderedBytes.getUint8(1)), closeTo(0.25, epsilon));
    expect(toFloat(renderedBytes.getUint8(2)), closeTo(0.75, epsilon));
    expect(toFloat(renderedBytes.getUint8(3)), closeTo(1.0, epsilon));
  });


  test('supported op shaders render correctly', () async {
    await _expectShadersRenderGreen('supported_op_shaders');
  });

  test('supported glsl op shaders render correctly', () async {
    await _expectShadersRenderGreen('supported_glsl_op_shaders');
  });
}

const double epsilon = 0.5 / 255.0;
double toFloat(int v) => v.toDouble() / 255.0;

// Iterates over all the .spv files in a folder and expects that they all only
// output the color green.
Future<void> _expectShadersRenderGreen(String leafFolderName) async {
  await for (final File spvFile in _spvFiles('supported_glsl_op_shaders')) {
    print(spvFile.path);
    final Uint8List spirvBytes = spvFile.readAsBytesSync();
    _expectShaderRendersGreen(spirvBytes);
  }
}

// Expects that a spirv shader only outputs the color green.
Future<void> _expectShaderRendersGreen(Uint8List spirvBytes) async {
  final FragmentShader shader = FragmentShader(spirv: spirvBytes.buffer);
  final PictureRecorder recorder = PictureRecorder();
  final Canvas canvas = Canvas(recorder);
  final Paint paint = Paint()..shader = shader;
  canvas.drawPaint(paint);
  final Picture picture = recorder.endRecording();
  final Image image = await picture.toImage(
    _shaderImageDimension,
    _shaderImageDimension,
  );
  final ByteData renderedBytes = await image.toByteData();
  for (final int color in renderedBytes.buffer.asUint32List()) {
    expect(color, _greenColor);
  }
}

// Gets the .spv files in a generated folder.
Stream<File> _spvFiles(String leafFolderName) async* {
  final Directory dir = _createDirectory(leafFolderName);
  await for (final FileSystemEntity entry in dir.list()) {
    if (entry is! File) {
      continue;
    }
    final File file = entry as File;
    if (path.extension(file.path) != '.spv') {
      continue;
    }
    yield file;
  }
}

Directory _createDirectory(String leafFolderName) {
  return Directory(path.joinAll(_basePathChunks + <String>[leafFolderName]));
}

const List<String> _basePathChunks = <String>[
  'out',
  'host_debug_unopt',
  'gen',
  'flutter',
  'lib',
  'spirv',
  'test',
];

// Arbitrary, but needs to be greater than 1 for frag coord tests.
const int _shaderImageDimension = 4;

// TODO(clocksmith): Why is this ARGB when Image.toByteData() returns RGBA?
const int _greenColor = 0xFF00FF00;