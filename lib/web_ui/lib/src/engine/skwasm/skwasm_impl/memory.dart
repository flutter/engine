// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:js_interop';

typedef DisposeFunction<T extends NativeType> = void Function(Pointer<T>);
JSFunction createSkwasmFinalizer<T extends NativeType>(DisposeFunction<T> dispose) {
  return ((JSNumber address) => 
    dispose(Pointer<Void>.fromAddress(address.toDart.toInt()).cast<T>())
  ).toJS;
}
