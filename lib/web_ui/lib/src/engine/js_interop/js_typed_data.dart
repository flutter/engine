// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:js_interop';

@JS()
@staticInterop
class ArrayBuffer {}

@JS()
@staticInterop
class TypedArray {}

extension TypedArrayExtension on TypedArray {
  external void set(Uint8Array source, JSNumber start);
  external JSNumber get length;
}

@JS()
@staticInterop
class Uint8Array extends TypedArray {
  external factory Uint8Array._(JSAny bufferOrLength);
}

Uint8Array createUint8ArrayFromBuffer(ArrayBuffer buffer) => Uint8Array._(buffer as JSObject);
Uint8Array createUint8ArrayFromLength(int length) => Uint8Array._(length.toJS);
