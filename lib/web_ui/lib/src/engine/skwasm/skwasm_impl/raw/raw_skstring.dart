// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@DefaultAsset('skwasm')
library skwasm_impl;

import 'dart:convert';
import 'dart:ffi';

final class RawSkString extends Opaque {}
typedef SkStringHandle = Pointer<RawSkString>;

@Native<SkStringHandle Function(Size)>(symbol: 'skString_allocate', isLeaf: true)
external SkStringHandle skStringAllocate(int size);

@Native<Pointer<Int8> Function(SkStringHandle)>(symbol: 'skString_getData', isLeaf: true)
external Pointer<Int8> skStringGetData(SkStringHandle handle);

@Native<Void Function(SkStringHandle)>(symbol: 'skString_free', isLeaf: true)
external void skStringFree(SkStringHandle handle);

SkStringHandle skStringFromDartString(String string) {
  final List<int> rawUtf8Bytes = utf8.encode(string);
  final SkStringHandle stringHandle = skStringAllocate(rawUtf8Bytes.length);
  final Pointer<Int8> stringDataPointer = skStringGetData(stringHandle);
  for (int i = 0; i < rawUtf8Bytes.length; i++) {
    stringDataPointer[i] = rawUtf8Bytes[i];
  }
  return stringHandle;
}
