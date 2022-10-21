// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'assets.dart';

abstract class FontCollection {
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily});

  /// Completes when fonts from FontManifest.json have been downloaded.
  Future<void> downloadAssetFonts(AssetManager assetManager);

  /// Registeres fonts that have been downloaded but not yet registered with the
  /// TypefaceFontProvider.
  ///
  /// downloading of fonts happens separately from registering of fonts so that
  /// the download step can happen concurrently with the download of wasm.
  void registerDownloadedFonts();
  FutureOr<void> debugDownloadTestFonts();
  void clear();
}
