// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:ffi';
import 'dart:js_interop';

import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

class SkwasmFontCollection implements FlutterFontCollection {
  SkwasmFontCollection() : _handle = fontCollectionCreate();

  final FontCollectionHandle _handle;

  @override
  void clear() {
    // TODO(jacksongardner): implement clear
  }

  @override
  FutureOr<void> debugDownloadTestFonts() {
    // TODO(jacksongardner): implement debugDownloadTestFonts
  }

  @override
  Future<void> downloadAssetFonts(AssetManager assetManager) async {
    final FontManifest manifest = await fetchFontManifest(assetManager);
    final List<Future<void>> fontFutures = <Future<void>>[];
    for (final FontFamily family in manifest.families) {
      final List<int> rawUtf8Bytes = utf8.encode(family.name);
      final SkStringHandle stringHandle = skStringAllocate(rawUtf8Bytes.length);
      final Pointer<Int8> stringDataPointer = skStringGetData(stringHandle);
      for (int i = 0; i < rawUtf8Bytes.length; i++) {
        stringDataPointer[i] = rawUtf8Bytes[i];
      }
      for (final FontAsset fontAsset in family.fontAssets) {
        fontFutures.add(_downloadFontAsset(fontAsset.asset, stringHandle));
      }
      skStringFree(stringHandle);
    }
    await Future.wait(fontFutures);
  }

  Future<void> _downloadFontAsset(String assetName, SkStringHandle familyNameHandle) async {
    final HttpFetchResponse response = await assetManager.loadAsset(assetName);
    if (!response.hasPayload) {
      printWarning('Failed to load font "$assetName", font file not found.');
      return;
    }
    int length = 0;
    final List<Uint8Array> chunks = <Uint8Array>[];
    await response.read((Uint8Array chunk) {
      length += chunk.length.toDart.toInt();
      chunks.add(chunk);
    });
    final SkDataHandle fontData = skDataCreate(length);
    int dataAddress = skDataGetPointer(fontData).cast<Int8>().address;
    final Uint8Array wasmMemory = createUint8ArrayFromBuffer(skwasmInstance.wasmMemory.buffer);
    for (final Uint8Array chunk in chunks) {
      wasmMemory.set(chunk, dataAddress.toJS);
      dataAddress += chunk.length.toDart.toInt();
    }
    fontCollectionRegisterFont(_handle, fontData, familyNameHandle);
    skDataDispose(fontData);
  }

  @override
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily}) async {
    final SkDataHandle dataHandle = skDataCreate(list.length);
    final Pointer<Int8> dataPointer = skDataGetPointer(dataHandle).cast<Int8>();
    for (int i = 0; i < list.length; i++) {
      dataPointer[i] = list[i];
    }
    if (fontFamily != null) {
      final List<int> rawUtf8Bytes = utf8.encode(fontFamily);
      final SkStringHandle stringHandle = skStringAllocate(rawUtf8Bytes.length);
      final Pointer<Int8> stringDataPointer = skStringGetData(stringHandle);
      for (int i = 0; i < rawUtf8Bytes.length; i++) {
        stringDataPointer[i] = rawUtf8Bytes[i];
      }
      fontCollectionRegisterFont(_handle, dataHandle, stringHandle);
      skStringFree(stringHandle);
    } else {
      fontCollectionRegisterFont(_handle, dataHandle, nullptr);
    }
    skDataDispose(dataHandle);
  }

  @override
  void registerDownloadedFonts() {
    // TODO(jacksongardner): implement registerDownloadedFonts
  }
}
