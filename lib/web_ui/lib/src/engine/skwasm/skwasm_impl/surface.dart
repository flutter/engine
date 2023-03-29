// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:js_interop';
import 'dart:js_util';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

class SkwasmSurface {
  factory SkwasmSurface(String canvasQuerySelector) {
    final SurfaceHandle surfaceHandle = withStackScope((StackScope scope) {
      final Pointer<Int8> pointer = scope.convertStringToNative(canvasQuerySelector);
      return surfaceCreateFromCanvas(pointer);
    });
    final SkwasmSurface surface = SkwasmSurface._fromHandle(surfaceHandle);
    surface._initialize();
    return surface;
  }

  SkwasmSurface._fromHandle(this._handle);
  final SurfaceHandle _handle;
  late final OnRenderCallbackHandle _callbackHandle;

  void _initialize() {
    _callbackHandle = 
      OnRenderCallbackHandle.fromAddress(
        skwasmInstance.addFunction(allowInterop(_onRender), 'vi').toDart.toInt()
      );
    surfaceSetOnRenderCallback(_handle, _callbackHandle);
  }

  void setSize(int width, int height) =>
    surfaceSetCanvasSize(_handle, width, height);

  void renderPicture(SkwasmPicture picture) =>
    surfaceRenderPicture(_handle, picture.handle);

  void _onRender(int renderId) {
    print('Render complete with ID $renderId');
  }

  void dispose() {
    surfaceDestroy(_handle);
    skwasmInstance.removeFunction(_callbackHandle.address.toJS);
  }
}
