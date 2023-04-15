// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:ffi';

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
