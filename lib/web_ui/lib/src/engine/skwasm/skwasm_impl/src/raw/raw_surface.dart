import 'dart:ffi';
import 'raw_picture.dart';

class RawSurface extends Opaque {}
typedef SurfaceHandle = Pointer<RawSurface>;

@FfiNative<SurfaceHandle Function(Pointer<Int8>)>(
  'skwasm.surface_createFromCanvas',
  isLeaf: true)
external SurfaceHandle surfaceCreateFromCanvas(Pointer<Int8> querySelector);

@FfiNative<Void Function(SurfaceHandle)>(
  'skwasm.surface_destroy',
  isLeaf: true)
external void surfaceDestroy(SurfaceHandle surface);

@FfiNative<Void Function(SurfaceHandle, Int, Int)>(
  'skwasm.surface_setCanvasSize',
  isLeaf: true)
external void surfaceSetCanvasSize(
  SurfaceHandle surface, 
  int width, 
  int height
);

@FfiNative<Void Function(SurfaceHandle, PictureHandle)>(
  'skwasm.surface_renderPicture',
  isLeaf: true)
external void surfaceRenderPicture(SurfaceHandle surface, PictureHandle picture);
