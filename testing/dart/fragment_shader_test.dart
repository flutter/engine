// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';

import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as path;

void main() {
  test('throws exception for invalid shader', () {
    final ByteBuffer invalidBytes = Uint8List.fromList(<int>[1, 2, 3, 4, 5]).buffer;
    expect(() => FragmentShader(spirv: invalidBytes), throws);
  });

  test('simple shader renders correctly', () async {
    final Uint8List shaderBytes = await _createFile('general_shaders', 'simple.spv').readAsBytes();
    _expectShaderRendersGreen(shaderBytes);
  });

  test('shader with uniforms renders and updates correctly', () async {
    final Uint8List shaderBytes = await _createFile('general_shaders', 'uniforms.spv').readAsBytes();
    final FragmentShader shader = FragmentShader(spirv: shaderBytes.buffer);

    shader.update(floatUniforms: Float32List.fromList(<double>[
      0.0,  // iFloatUniform
      0.25, // iVec2Uniform.x
      0.75, // iVec2Uniform.y
      0.0,    // iMat2Uniform[0][0]
      0.0,    // iMat2Uniform[0][1]
      0.0,    // iMat2Uniform[1][0]
      1.0,    // iMat2Uniform[1][1]
    ]));

    final ByteData renderedBytes = (await _imageByteDataFromShader(
      shader: shader,
    ))!;

    expect(toFloat(renderedBytes.getUint8(0)), closeTo(0.0, epsilon));
    expect(toFloat(renderedBytes.getUint8(1)), closeTo(0.25, epsilon));
    expect(toFloat(renderedBytes.getUint8(2)), closeTo(0.75, epsilon));
    expect(toFloat(renderedBytes.getUint8(3)), closeTo(1.0, epsilon));
  });

  // Test all supported GLSL ops. See lib/spirv/lib/src/constants.dart
  _expectShadersRenderGreen('supported_glsl_op_shaders');

  // Test all supported instructions. See lib/spirv/lib/src/constants.dart
  _expectShadersRenderGreen('supported_op_shaders');
}

// Expect that all of the spirv shaders in this folder render green.
// Keeping the outer loop of the test synchronous allows for easy printing
// of the file name within the test case.
void _expectShadersRenderGreen(String leafFolderName) {
  final List<File> files = _spvFiles(leafFolderName);

  test('$leafFolderName has shaders', () {
    expect(files.length > 0, true);
  });

  for (final File spvFile in files) {
    test('${path.basenameWithoutExtension(spvFile.path)} renders correctly', () {
      _expectShaderRendersGreen(spvFile.readAsBytesSync());
    });
  }
}

// Expects that a spirv shader only outputs the color green.
Future<void> _expectShaderRendersGreen(Uint8List spirvBytes) async {
  final FragmentShader shader = FragmentShader(spirv: spirvBytes.buffer);
  final ByteData renderedBytes = (await _imageByteDataFromShader(
    shader: shader,
    imageDimension: _shaderImageDimension,
  ))!;
  for (final int color in renderedBytes.buffer.asUint32List()) {
    expect(toHexString(color), toHexString(_greenColor.value));
  }
}

Future<ByteData?> _imageByteDataFromShader({
  required FragmentShader shader,
  int imageDimension = 100,
}) async {
  final PictureRecorder recorder = PictureRecorder();
  final Canvas canvas = Canvas(recorder);
  final Paint paint = Paint()..shader = shader;
  canvas.drawPaint(paint);
  final Picture picture = recorder.endRecording();
  final Image image = await picture.toImage(
    imageDimension,
    imageDimension,
  );
  return image.toByteData();
}

// Gets the .spv files in a generated folder in alphabetical order.
List<File> _spvFiles(String leafFolderName) {
  return _createDirectory(leafFolderName).listSync()
    .where((FileSystemEntity entry) => path.extension(entry.path) == '.spv')
    .map((FileSystemEntity entry) => entry as File)
    .toList()
    ..sort((File a, File b) => a.path.compareTo(b.path));
}

// Creates the directory that contains shader test files.
Directory _createDirectory(String leafFolderName) {
  return Directory(path.joinAll(_basePathChunks + <String>[leafFolderName]));
}

File _createFile(String folderName, String fileName) {
  return File(path.joinAll(_basePathChunks + <String>[folderName, fileName]));
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

const Color _greenColor = Color(0xFF00FF00);

// Precision for checking uniform values.
const double epsilon = 0.5 / 255.0;

// Maps an int value from 0-255 to a double value of 0.0 to 1.0.
double toFloat(int v) => v.toDouble() / 255.0;

String toHexString(int color) => '#${color.toRadixString(16)}';