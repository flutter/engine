// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of dart.ui;

/// Load the asset bytes specified by [assetKey].
///
/// The [assetKey] is generally the filepath of the asset which is bundled
/// into the flutter application. This API is not supported on the Web.
///
/// If the [assetKey] does not correspond to a real asset, returns `null`.
ByteData? loadAsset(String assetKey) {
  ByteData? result;
  _loadAsset(assetKey, (Uint8List bytes) {
    result = bytes.buffer.asByteData();
  });
  return result;
}

void _loadAsset(String asseyKey, void Function(Uint8List) onData) native 'loadAssetBytes';
