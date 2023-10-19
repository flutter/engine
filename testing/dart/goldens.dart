// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';

import 'package:path/path.dart' as path;
import 'package:skia_gold_client/skia_gold_client.dart';

import 'impeller_enabled.dart';

const String _kSkiaGoldWorkDirectoryKey = 'kSkiaGoldWorkDirectory';

/// A helper for doing image comparison (golden) tests.
///
/// Contains utilities for comparing two images in memory that are expected to
/// be identical, or for adding images to Skia gold for comparison.
class ImageComparer {
  ImageComparer._({
    required this.testSuiteName,
    required SkiaGoldClient client,
  }) : _client = client;

  /// Creates an image comparer and authorizes.
  static Future<ImageComparer> create({required String testSuiteName}) async {
    const String workDirectoryPath =
        String.fromEnvironment(_kSkiaGoldWorkDirectoryKey);
    if (workDirectoryPath.isEmpty) {
      throw UnsupportedError(
          'Using ImageComparer requries defining kSkiaGoldWorkDirectoryKey.');
    }

    final Directory workDirectory = Directory(workDirectoryPath)..createSync();
    final Map<String, String> dimensions = <String, String>{
      'impeller_enabled': impellerEnabled.toString(),
    };
    final SkiaGoldClient client = isSkiaGoldClientAvailable
        ? SkiaGoldClient(workDirectory, dimensions: dimensions)
        : _FakeSkiaGoldClient(workDirectory, dimensions);

    await client.auth();
    print('Auth done!');
    return ImageComparer._(testSuiteName: testSuiteName, client: client);
  }

  final SkiaGoldClient _client;

  /// A unique name for the suite under test, e.g. `canvas_test`.
  final String testSuiteName;

  /// Adds an [Image] to Skia Gold for comparison.
  ///
  /// The [fileName] must be unique per [testSuiteName].
  Future<void> addGoldenImage(Image image, String fileName) async {
    final ByteData data =
        (await image.toByteData(format: ImageByteFormat.png))!;

    final File file = File(path.join(_client.workDirectory.path, fileName))
      ..writeAsBytesSync(data.buffer.asUint8List());
      print('Adding image $testSuiteName $file');
    await _client.addImg(
      testSuiteName,
      file,
      screenshotSize: image.width * image.height,
    ).catchError((dynamic error) {
      print('Skia gold comparison failed: $error');
      throw Exception('Failed comparison: $testSuiteName/$fileName');
    });
    print('Added image!');
  }

  Future<bool> fuzzyCompareImages(Image golden, Image testImage) async {
    if (golden.width != testImage.width || golden.height != testImage.height) {
      return false;
    }
    int getPixel(ByteData data, int x, int y) =>
        data.getUint32((x + y * golden.width) * 4);
    final ByteData goldenData = (await golden.toByteData())!;
    final ByteData testImageData = (await testImage.toByteData())!;
    for (int y = 0; y < golden.height; y++) {
      for (int x = 0; x < golden.width; x++) {
        if (getPixel(goldenData, x, y) != getPixel(testImageData, x, y)) {
          return false;
        }
      }
    }
    return true;
  }
}

// TODO(dnfield): add local comparison against baseline,
// https://github.com/flutter/flutter/issues/136831
class _FakeSkiaGoldClient implements SkiaGoldClient {
  _FakeSkiaGoldClient(this.workDirectory, this.dimensions);

  @override
  final Directory workDirectory;

  @override
  final Map<String, String> dimensions;

  @override
  Future<void> auth() async {}

  @override
  Future<void> addImg(
    String testName,
    File goldenFile, {
    double differentPixelsRate = 0.01,
    int pixelColorDelta = 0,
    required int screenshotSize,
  }) async {}

  @override
  dynamic noSuchMethod(Invocation invocation) {
    throw UnimplementedError(invocation.memberName.toString().split('"')[1]);
  }
}
