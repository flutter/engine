import 'dart:ffi';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';

class Surface {
  factory Surface(String canvasQuerySelector) {
    final SurfaceHandle surfaceHandle = withStackScope((StackScope scope) {
      final Pointer<Int8> pointer = scope.convertString(canvasQuerySelector);
      return surfaceCreateFromCanvas(pointer);
    });
    return Surface._fromHandle(surfaceHandle);
  }

  Surface._fromHandle(this._handle);
  final SurfaceHandle _handle;

  void setSize(int width, int height) =>
    surfaceSetCanvasSize(_handle, width, height);

  void renderPicture(SkwasmPicture picture) =>
    surfaceRenderPicture(_handle, picture.handle);
}
