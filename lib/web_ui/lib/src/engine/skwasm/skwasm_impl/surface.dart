// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';
import 'dart:js_interop';
import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmSurface {
  factory SkwasmSurface() {
    final SurfaceHandle surfaceHandle = withStackScope((StackScope scope) {
      return surfaceCreate();
    });
    final SkwasmSurface surface = SkwasmSurface._fromHandle(surfaceHandle);
    surface._initialize();
    return surface;
  }

  SkwasmSurface._fromHandle(this.handle) : threadId = surfaceGetThreadId(handle);
  final SurfaceHandle handle;
  OnRenderCallbackHandle _callbackHandle = nullptr;
  final Map<int, Completer<int>> _pendingCallbacks = <int, Completer<int>>{};

  final int threadId;

  int acquireObjectId() => skwasmInstance.skwasmGenerateUniqueId().toDart.toInt();

  void _initialize() {
    _callbackHandle =
      OnRenderCallbackHandle.fromAddress(
        skwasmInstance.addFunction(
          _callbackHandler.toJS,
          'vii'.toJS
        ).toDart.toInt()
      );
    surfaceSetCallbackHandler(handle, _callbackHandle);
  }

  Future<DomImageBitmap> renderPicture(SkwasmPicture picture) async {
    final int callbackId = surfaceRenderPicture(handle, picture.handle);
    await _registerCallback(callbackId);
    return skwasmInstance.skwasmGetObject(callbackId.toJS) as DomImageBitmap;
  }

  Future<ByteData> rasterizeImage(SkwasmImage image, ui.ImageByteFormat format) async {
    final int callbackId = surfaceRasterizeImage(
      handle,
      image.handle,
      format.index,
    );
    final int context = await _registerCallback(callbackId);
    final SkDataHandle dataHandle = SkDataHandle.fromAddress(context);
    final int byteCount = skDataGetSize(dataHandle);
    final Pointer<Uint8> dataPointer = skDataGetConstPointer(dataHandle).cast<Uint8>();
    final Uint8List output = Uint8List(byteCount);
    for (int i = 0; i < byteCount; i++) {
      output[i] = dataPointer[i];
    }
    skDataDispose(dataHandle);
    return ByteData.sublistView(output);
  }

  Future<int> _registerCallback(int callbackId) {
    final Completer<int> completer = Completer<int>();
    _pendingCallbacks[callbackId] = completer;
    return completer.future;
  }

  void _callbackHandler(JSNumber jsCallbackId, JSNumber jsPointer) {
    final int callbackId = jsCallbackId.toDart.toInt();
    final Completer<int> completer = _pendingCallbacks.remove(callbackId)!;
    completer.complete(jsPointer.toDart.toInt());
  }

  void dispose() {
    surfaceDestroy(handle);
    skwasmInstance.removeFunction(_callbackHandle.address.toJS);
  }
}
