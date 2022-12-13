import 'dart:ffi';

import 'raw_canvas.dart';
import 'raw_geometry.dart';

class RawPictureRecorder extends Opaque {}
typedef PictureRecorderHandle = Pointer<RawPictureRecorder>;

class RawPicture extends Opaque {}
typedef PictureHandle = Pointer<RawPicture>;

@FfiNative<PictureRecorderHandle Function()>(
  'skwasm.pictureRecorder_create',
  isLeaf: true)
external PictureRecorderHandle pictureRecorderCreate();

@FfiNative<Void Function(PictureRecorderHandle)>(
  'skwasm.pictureRecorder_destroy',
  isLeaf: true)
external void pictureRecorderDestroy(PictureRecorderHandle picture);

@FfiNative<CanvasHandle Function(PictureRecorderHandle, RawRect)>(
  'skwasm.pictureRecorder_beginRecording',
  isLeaf: true)
external CanvasHandle pictureRecorderBeginRecording(
    PictureRecorderHandle picture, RawRect cullRect);

@FfiNative<PictureHandle Function(PictureRecorderHandle)>(
  'skwasm.pictureRecorder_endRecording',
  isLeaf: true)
external PictureHandle pictureRecorderEndRecording(PictureRecorderHandle picture);

@FfiNative<Void Function(PictureHandle)>(
  'skwasm.pictureRecorder_dispose',
  isLeaf: true)
external void pictureDispose(PictureHandle handle);

@FfiNative<Uint32 Function(PictureHandle)>(
  'skwasm.pictureRecorder_approximateBytesUsed',
  isLeaf: true)
external int pictureApproximateBytesUsed(PictureHandle handle);
