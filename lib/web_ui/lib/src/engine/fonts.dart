// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'assets.dart';

abstract class FontCollection {
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily});
  Future<void> downloadAssetFonts(AssetManager assetManager);
  void registerDownloadedFonts();
  FutureOr<void> debugDownloadTestFonts();
  void clear();
}
