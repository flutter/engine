// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

class PlatformIsolate {
  static Future<Isolate> spawn<T>(
      void entryPoint(T message), T message,
      /*{bool paused = false,
      bool errorsAreFatal = true,
      SendPort? onExit,
      SendPort? onError,
      @Since("2.3") String? debugName}*/) {
    // return _spawn(entryPoint, message);
    return Isolate.spawn<T>(entryPoint, message, debugName: "PlatformIsolate");
  }

  // @Native<Handle Function(Handle, Handle)>(symbol: 'PlatformIsolateNativeApi::Spawn')
  // external static Future<Isolate> _spawn<T>(
  //     void entryPoint(T message), T message);
}
