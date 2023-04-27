// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';
import 'dart:js_interop';

import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

// This URL was found by using the Google Fonts Developer API to find the URL
// for Roboto. The API warns that this URL is not stable. In order to update
// this, list out all of the fonts and find the URL for the regular
// Roboto font. The API reference is here:
// https://developers.google.com/fonts/docs/developer_api
const String _robotoUrl =
    'https://fonts.gstatic.com/s/roboto/v20/KFOmCnqEu92Fr1Me5WZLCzYlKw.ttf';

class SkwasmFontCollection implements FlutterFontCollection {
  SkwasmFontCollection() : handle = fontCollectionCreate();

  FontCollectionHandle handle;

  @override
  void clear() {
    fontCollectionDispose(handle);
    handle = fontCollectionCreate();
  }

  @override
  Future<AssetFontsResult> loadAssetFonts(FontManifest manifest) async {
    final List<Future<void>> fontFutures = <Future<void>>[];
    final List<String> loadedFonts = <String>[];
    final Map<String, FontLoadError> fontFailures = <String, FontLoadError>{};

    /// We need a default fallback font for Skwasm, in order to avoid crashing
    /// while laying out text with an unregistered font. We chose Roboto to
    /// match Android.
    if (!manifest.families.any((FontFamily family) => family.name == 'Roboto')) {
      manifest.families.add(
        FontFamily('sans-serif', <FontAsset>[FontAsset(_robotoUrl, <String, String>{})])
      );
    }

    // We can't restore the pointers directly due to a bug in dart2wasm
    // https://github.com/dart-lang/sdk/issues/52142
    final List<int> familyHandles = <int>[];
    for (final FontFamily family in manifest.families) {
      final SkStringHandle familyNameHandle = skStringFromDartString(family.name);
      familyHandles.add(familyNameHandle.address);
      for (final FontAsset fontAsset in family.fontAssets) {
        fontFutures.add(() async {
          final FontLoadError? error = await _downloadFontAsset(fontAsset, familyNameHandle);
          if (error == null) {
            loadedFonts.add(fontAsset.asset);
          } else {
            fontFailures[fontAsset.asset] = error;
          }
        }());
      }
    }

    await Future.wait(fontFutures);

    // Wait until all the downloading and registering is complete before
    // freeing the handles to the family name strings.
    familyHandles
      .map((int address) => SkStringHandle.fromAddress(address))
      .forEach(skStringFree);
    return AssetFontsResult(loadedFonts, fontFailures);
  }

  Future<FontLoadError?> _downloadFontAsset(FontAsset asset, SkStringHandle familyNameHandle) async {
    final HttpFetchResponse response;
    try {
      response = await assetManager.loadAsset(asset.asset);
    } catch (error) {
      return FontDownloadError(assetManager.getAssetUrl(asset.asset), error);
    }
    if (!response.hasPayload) {
      return FontNotFoundError(assetManager.getAssetUrl(asset.asset));
    }
    int length = 0;
    final List<JSUint8Array1> chunks = <JSUint8Array1>[];
    await response.read((JSUint8Array1 chunk) {
      length += chunk.length.toDart.toInt();
      chunks.add(chunk);
    });
    final SkDataHandle fontData = skDataCreate(length);
    int dataAddress = skDataGetPointer(fontData).cast<Int8>().address;
    final JSUint8Array1 wasmMemory = createUint8ArrayFromBuffer(skwasmInstance.wasmMemory.buffer);
    for (final JSUint8Array1 chunk in chunks) {
      wasmMemory.set(chunk, dataAddress.toJS);
      dataAddress += chunk.length.toDart.toInt();
    }
    final bool result = fontCollectionRegisterFont(handle, fontData, familyNameHandle);
    skDataDispose(fontData);
    if (!result) {
      return FontInvalidDataError(assetManager.getAssetUrl(asset.asset));
    }
    return null;
  }

  @override
  Future<bool> loadFontFromList(Uint8List list, {String? fontFamily}) async {
    final SkDataHandle dataHandle = skDataCreate(list.length);
    final Pointer<Int8> dataPointer = skDataGetPointer(dataHandle).cast<Int8>();
    for (int i = 0; i < list.length; i++) {
      dataPointer[i] = list[i];
    }
    bool success;
    if (fontFamily != null) {
      final SkStringHandle familyHandle = skStringFromDartString(fontFamily);
      success = fontCollectionRegisterFont(handle, dataHandle, familyHandle);
      skStringFree(familyHandle);
    } else {
      success = fontCollectionRegisterFont(handle, dataHandle, nullptr);
    }
    skDataDispose(dataHandle);
    return success;
  }
}
