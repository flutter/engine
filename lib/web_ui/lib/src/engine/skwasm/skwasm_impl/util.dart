// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection';
import 'dart:ffi';

class FfiUint8PointerList extends ListBase<int> {
  FfiUint8PointerList(this.pointer, this._length);

  final Pointer<Uint8> pointer;
  final int _length;

  @override
  int get length => _length;

  @override
  set length(int length) => throw Exception('Cannot resize ffi pointer list');

  @override
  int operator [](int index) {
    return pointer[index];
  }

  @override
  void operator []=(int index, int value) {
    pointer[index] = value;
  }
}
