// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';

class FakeAssetManager implements AssetManager {
  @override
  String get assetsDir => 'assets';

  @override
  String getAssetUrl(String asset) => asset;

  @override
  Future<ByteData> load(String assetKey) async {
    final ByteData? data = _assetMap[assetKey];
    if (data == null) {
      throw HttpFetchNoPayloadError(assetKey, status: 404);
    }
    return data;
  }

  @override
  Future<HttpFetchResponse> loadAsset(String assetKey) async {
    final ByteData? data = _assetMap[assetKey];
    if (data == null) {
      return MockHttpFetchResponse(
        url: getAssetUrl(assetKey),
        status: 404,
      );
    }
    return MockHttpFetchResponse(
        url: getAssetUrl(assetKey),
        status: 200,
        payload: MockHttpFetchPayload(
          byteBuffer: data.buffer,
        ),
      );
  }

  void setMockAssetData(String assetKey, ByteData assetData) {
    _assetMap[assetKey] = assetData;
  }

  void setMockAssetStringAsUtf8Data(String assetKey, String stringData) {
    final List<int> byteList = utf8.encode(stringData);
    final Uint8List byteData = Uint8List(byteList.length);
    byteData.setAll(0, byteList);
    setMockAssetData(assetKey, ByteData.sublistView(byteData));
  }

  void clearMocks() {
    _assetMap.clear();
  }

  final Map<String, ByteData> _assetMap = <String, ByteData>{};
}

FakeAssetManager fakeAssetManager = FakeAssetManager();

void setUpStandardMocks(FakeAssetManager assetManager) {
  const String ahemFontFamily = 'Ahem';
  const String ahemFontUrl = '/assets/fonts/ahem.ttf';
  const String robotoFontFamily = 'Roboto';
  const String robotoTestFontUrl = '/assets/fonts/Roboto-Regular.ttf';

  assetManager.setMockAssetStringAsUtf8Data('AssetManifest.json', '{}');
  assetManager.setMockAssetStringAsUtf8Data('FontManifest.json',
  '''
  [
   {
      "family":"$robotoFontFamily",
      "fonts":[{"asset":"$robotoTestFontUrl"}]
   },
   {
      "family":"$ahemFontFamily",
      "fonts":[{"asset":"$ahemFontUrl"}]
   }
  ]''');
}
