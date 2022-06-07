// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Create a buffer from the asset with key [assetKey].
///
/// Throws an [Exception] if the asset does not exist.
Future<ByteData> loadAsset(String assetKey) {
  return webOnlyAssetManager.load(assetKey);
}
