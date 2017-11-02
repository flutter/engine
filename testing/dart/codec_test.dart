// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';
import 'dart:ui' as ui;
import 'dart:typed_data';

import 'package:test/test.dart';
import 'package:path/path.dart' as path;

void main() {

  test('Do something', () async {
    Uint8List data = await _getSkiaResource('alphabetAnim.gif').readAsBytes();
    final Completer<ui.Codec> completer = new Completer<ui.Codec>();
    ui.instantiateImageCodec(data, completer.complete);
    ui.Codec codec = await completer.future;
  });
}

/// Returns a File handle to a file in the skia/resources directory.
File _getSkiaResource(String fileName) {
  // As Platform.script is not working for flutter_tester
  // (https://github.com/flutter/flutter/issues/12847), this is currently
  // assuming the curent working directory is engine/src.
  // This is fragile and should be changed once the Platform.script issue is
  // resolved.
  String assetPath =
    path.join('third_party', 'skia', 'resources', fileName);
  return new File(assetPath);
}
