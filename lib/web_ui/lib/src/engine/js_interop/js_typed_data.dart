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
  external void set(JSAny source, JSNumber start);
  external JSNumber get length;
}

@JS()
@staticInterop
class Uint8Array extends TypedArray {
  external factory Uint8Array(ArrayBuffer buffer);
}
