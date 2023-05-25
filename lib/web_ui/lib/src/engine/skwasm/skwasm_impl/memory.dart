// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:js_interop';

import 'package:ui/src/engine.dart';

abstract class SkwasmObjectWrapper<T extends NativeType> {
  Pointer<T> get handle;
}

typedef DisposeFunction<T extends NativeType> = void Function(Pointer<T>);

class SkwasmFinalizationRegistry<T extends NativeType> {
  SkwasmFinalizationRegistry(DisposeFunction<T> dispose)
    : registry = DomFinalizationRegistry(((JSNumber address) =>
      dispose(Pointer<T>.fromAddress(address.toDart.toInt()))
    ).toJS);

  final DomFinalizationRegistry registry;

  void register(SkwasmObjectWrapper<T> wrapper) {
    registry.register(wrapper, wrapper.handle.address, wrapper);
  }

  void unregister(SkwasmObjectWrapper<T> wrapper) {
    registry.unregister(wrapper);
  }
}
