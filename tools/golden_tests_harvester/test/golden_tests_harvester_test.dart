// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;

import 'package:golden_tests_harvester/golden_tests_harvester.dart';
import 'package:litetest/litetest.dart';
import 'package:path/path.dart' as p;

void main() async {
  Future<void> withTempDirectory(FutureOr<void> Function(io.Directory) callback) async {
    final io.Directory tempDirectory = await io.Directory.systemTemp.createTemp('golden_tests_harvester_test.');
    try {
      await callback(tempDirectory);
    } finally {
      await tempDirectory.delete(recursive: true);
    }
  }

  test('should require a file named "digests.json" in the working directory', () async {
    await withTempDirectory((io.Directory tempDirectory) async {
      final StringSink stderr = StringBuffer();
      try {
        await harvest(
          workDirectory: tempDirectory,
          addImg: _alwaysThrowsAddImg,
          stderr: stderr,
        );
        fail('Expected a FileSystemException');
      } on io.FileSystemException catch (e) {
        expect(e.path, contains('digests.json'));
        expect(stderr.toString(), isEmpty);
      }
    });
  });

  test('should throw if "digests.json" is in an unexpected format', () async {
    await withTempDirectory((io.Directory tempDirectory) async {
      final StringSink stderr = StringBuffer();
      final io.File digestsFile = io.File(p.join(tempDirectory.path, 'digests.json'));
      await digestsFile.writeAsString('{"dimensions": "not a map", "entries": []}');
      try {
        await harvest(
          workDirectory: tempDirectory,
          addImg: _alwaysThrowsAddImg,
          stderr: stderr,
        );
        fail('Expected a FormatException');
      } on FormatException catch (e) {
        expect(e.message, contains('dimensions'));
        expect(stderr.toString(), isEmpty);
      }
    });
  });

  test('should fail eagerly if addImg fails', () async {
    await withTempDirectory((io.Directory tempDirectory) async {
      final io.File digestsFile = io.File(p.join(tempDirectory.path, 'digests.json'));
      final StringSink stderr = StringBuffer();
      await digestsFile.writeAsString('''
        {
          "dimensions": {},
          "entries": [
            {
              "filename": "test_name_1.png",
              "width": 100,
              "height": 100,
              "maxDiffPixelsPercent": 0.01,
              "maxColorDelta": 0
            }
          ]
        }
      ''');
      try {
        await harvest(
          workDirectory: tempDirectory,
          addImg: _alwaysThrowsAddImg,
          stderr: stderr,
        );
        fail('Expected an _IntentionalError');
      } on FailedComparisonException catch (e) {
        expect(e.testName, 'test_name_1.png');
        expect(stderr.toString(), contains('IntentionalError'));
      }
    });
  });

  test('should invoke addImg per test', () async {
    await withTempDirectory((io.Directory tempDirectory) async {
      final io.File digestsFile = io.File(p.join(tempDirectory.path, 'digests.json'));
      await digestsFile.writeAsString('''
        {
          "dimensions": {},
          "entries": [
            {
              "filename": "test_name_1.png",
              "width": 100,
              "height": 100,
              "maxDiffPixelsPercent": 0.01,
              "maxColorDelta": 0
            },
            {
              "filename": "test_name_2.png",
              "width": 200,
              "height": 200,
              "maxDiffPixelsPercent": 0.02,
              "maxColorDelta": 1
            }
          ]
        }
      ''');
      final List<String> addImgCalls = <String>[];
      final StringSink stderr = StringBuffer();
      await harvest(
        workDirectory: tempDirectory,
        addImg: (
          String testName,
          io.File goldenFile, {
          required int screenshotSize,
          double differentPixelsRate = 0.01,
          int pixelColorDelta = 0,
        }) async {
          addImgCalls.add('$testName $screenshotSize $differentPixelsRate $pixelColorDelta');
        },
        stderr: stderr,
      );
      expect(addImgCalls, <String>[
        'test_name_1.png 10000 0.01 0',
        'test_name_2.png 40000 0.02 1',
      ]);
    });
  
  });
}

final class _IntentionalError extends Error {}

Future<void> _alwaysThrowsAddImg(
  String testName,
  io.File goldenFile, {
  required int screenshotSize,
  double differentPixelsRate = 0.01,
  int pixelColorDelta = 0,
}) async {
  throw _IntentionalError();
}
