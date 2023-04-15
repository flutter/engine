// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:js_interop';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';

class FontAsset {
  FontAsset(this.asset, this.descriptors);

  String asset;
  Map<String, String> descriptors;
}

class FontFamily {
  FontFamily(this.name, this.fontAssets);

  String name;
  List<FontAsset> fontAssets;
}

class FontManifest {
  FontManifest(this.families);

  List<FontFamily> families;
}

Future<FontManifest> fetchFontManifest() async {
  final HttpFetchResponse response = await assetManager.loadAsset('FontManifest.json');
  if (!response.hasPayload) {
    printWarning('Font manifest does not exist at `${response.url}` - ignoring.');
    return FontManifest(<FontFamily>[]);
  }

  final Converter<List<int>, Object?> decoder = const Utf8Decoder().fuse(const JsonDecoder());
  Object? fontManifestJson;
  final Sink<List<int>> inputSink = decoder.startChunkedConversion(
    ChunkedConversionSink<Object?>.withCallback(
      (List<Object?> accumulated) {
        if (accumulated.length != 1) {
          throw AssertionError('There was a problem trying to load FontManifest.json');
        }
        fontManifestJson = accumulated.first;
      }
  ));
  await response.read((JSUint8Array chunk) => inputSink.add(chunk.toDart));
  inputSink.close();
  if (fontManifestJson == null) {
    throw AssertionError('There was a problem trying to load FontManifest.json');
  }
  final List<FontFamily> families = (fontManifestJson! as List<dynamic>).map(
    (dynamic fontFamilyJson) {
      final Map<String, dynamic> fontFamily = fontFamilyJson as Map<String, dynamic>;
      final String familyName = fontFamily.readString('family');
      final List<dynamic> fontAssets = fontFamily.readList('fonts');
      return FontFamily(familyName, fontAssets.map((dynamic fontAssetJson) {
        String? asset;
        final Map<String, String> descriptors = <String, String>{};
        for (final MapEntry<String, dynamic> descriptor in (fontAssetJson as Map<String, dynamic>).entries) {
          if (descriptor.key == 'asset') {
            asset = descriptor.value as String;
          } else {
            descriptors[descriptor.key] = descriptor.value as String;
          }
        }
        if (asset == null) {
          throw AssertionError("Invalid Font manifest, missing 'asset' key on font.");
        }
        return FontAsset(asset, descriptors);
      }).toList());
    }).toList();
  return FontManifest(families);
}

abstract class FlutterFontCollection {

  /// Fonts loaded with [loadFontFromList] do not need to be registered
  /// with [registerDownloadedFonts]. Fonts are both downloaded and registered
  /// with [loadFontFromList] calls.
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily});

  /// Completes when fonts from FontManifest.json have been downloaded.
  Future<void> downloadAssetFonts(AssetManager assetManager);

  /// Registers both downloaded fonts and fallback fonts with the TypefaceFontProvider.
  ///
  /// Downloading of fonts happens separately from registering of fonts so that
  /// the download step can happen concurrently with the initalization of the renderer.
  ///
  /// The correct order of calls to register downloaded fonts:
  /// 1) [downloadAssetFonts]
  /// 2) [registerDownloadedFonts]
  ///
  /// For fallbackFonts, call registerFallbackFont (see font_fallbacks.dart)
  /// for each fallback font before calling [registerDownloadedFonts]
  void registerDownloadedFonts();
  FutureOr<void> debugDownloadTestFonts();
  void clear();
}
