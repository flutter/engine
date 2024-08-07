// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import '../common/test_initialization.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

List<String>? testFiles;

Future<void> testMain() async {
  setUpUnitTests();

  Future<List<String>> createTestFiles() async {
    final HttpFetchResponse listingResponse = await httpFetch('/test_images/');
    final List<String> testFiles =
        (await listingResponse.json() as List<dynamic>).cast<String>();

    // Sanity-check the test file list. If suddenly test files are moved or
    // deleted, and the test server returns an empty list, or is missing some
    // important test files, we want to know.
    assert(testFiles.isNotEmpty);
    assert(testFiles.any((String testFile) => testFile.endsWith('.jpg')));
    assert(testFiles.any((String testFile) => testFile.endsWith('.png')));
    assert(testFiles.any((String testFile) => testFile.endsWith('.gif')));
    assert(testFiles.any((String testFile) => testFile.endsWith('.webp')));
    assert(testFiles.any((String testFile) => testFile.endsWith('.bmp')));

    return testFiles;
  }

  testFiles = await createTestFiles();

  for (final String testFile in testFiles!) {
    test('can detect image type of $testFile', () async {
      final HttpFetchResponse response =
          await httpFetch('/test_images/$testFile');

      if (!response.hasPayload) {
        throw Exception('Unable to fetch() image test file "$testFile"');
      }

      final Uint8List responseBytes = await response.asUint8List();

      // WebP files which are known to be animated.
      const List<String> animatedWebpFiles = <String>[
        'blendBG.webp',
        'required.webp',
        'stoplight_h.webp',
        'stoplight.webp',
      ];

      ImageType? expectedImageType;
      final String testFileExtension =
          testFile.substring(testFile.lastIndexOf('.') + 1);
      switch (testFileExtension) {
        case 'jpg':
          expectedImageType = ImageType.jpeg;
        case 'jpeg':
          expectedImageType = ImageType.jpeg;
        case 'gif':
          expectedImageType = ImageType.animatedGif;
        case 'webp':
          if (animatedWebpFiles.contains(testFile)) {
            expectedImageType = ImageType.animatedWebp;
          } else {
            expectedImageType = ImageType.webp;
          }
        case 'avif':
          expectedImageType = ImageType.avif;
        case 'bmp':
          expectedImageType = ImageType.bmp;
        case 'png':
          expectedImageType = ImageType.png;
      }

      expect(detectImageType(responseBytes), expectedImageType);
    });
  }
}
