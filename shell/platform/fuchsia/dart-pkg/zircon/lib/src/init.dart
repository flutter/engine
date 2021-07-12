// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

final _kLibZirconDartPath = '/pkg/lib/libzircon_ffi.so';

class _Bindings {
  static ZirconFFIBindings? _bindings;

  @pragma('vm:entry-point')
  static ZirconFFIBindings get() {
    if (_bindings == null) {
      final _dylib = DynamicLibrary.open(_kLibZirconDartPath);
      _bindings = ZirconFFIBindings(_dylib);
      // final initializer = _bindings?.zircon_dart_dl_initialize;
      // if (initializer == null ||
      //     initializer(NativeApi.initializeApiDLData) != 1) {
      //   throw Exception('Unable to initialize ZirconFFIBindings.');
      // }
    }
    return _bindings!;
  }
}

final ZirconFFIBindings zirconFFIBindings = _Bindings.get();
