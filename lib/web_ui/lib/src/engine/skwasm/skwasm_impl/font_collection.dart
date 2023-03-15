// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'dart:typed_data';

import 'package:ui/src/engine.dart';

class SkwasmFontCollection implements FontCollection {
  @override
  void clear() {
    // TODO: implement clear
    print('SkwasmFontCollection.clear()');
  }

  @override
  FutureOr<void> debugDownloadTestFonts() {
    // TODO: implement debugDownloadTestFonts
    print('SkwasmFontCollection.debugDownloadTestFonts()');
  }

  @override
  Future<void> downloadAssetFonts(AssetManager assetManager) async {
    // TODO: implement downloadAssetFonts
    print('SkwasmFontCollection.downloadAssetFonts($assetManager)');
  }

  @override
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily}) async {
    // TODO: implement loadFontFromList
    print('SkwasmFontCollection.loadFontFromList($list, $fontFamily)');
  }

  @override
  void registerDownloadedFonts() {
    // TODO: implement registerDownloadedFonts
    print('SkwasmFontCollection.registerDownloadedFonts()');
  }
}
