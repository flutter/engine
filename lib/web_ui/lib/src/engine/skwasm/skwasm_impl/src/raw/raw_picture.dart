import 'dart:ffi';

import 'raw_canvas.dart';
import 'raw_geometry.dart';
import 'raw_memory.dart';

class RawPictureRecorder extends Opaque {}

typedef PictureRecorderHandle = Pointer<RawPictureRecorder>;

class RawPicture extends Opaque {}

typedef PictureHandle = Pointer<RawPicture>;

@pragma('wasm:import', 'skwasm.pictureRecorder_create')
external PictureRecorderHandle pictureRecorder_create();

@pragma('wasm:import', 'skwasm.pictureRecorder_destroy')
external void pictureRecorder_destroy(PictureRecorderHandle picture);

@pragma('wasm:import', 'skwasm.pictureRecorder_beginRecording')
external CanvasHandle pictureRecorder_beginRecording(
    PictureRecorderHandle picture, RawRect cullRect);

@pragma('wasm:import', 'skwasm.pictureRecorder_endRecording')
external PictureHandle pictureRecorder_endRecording(
    PictureRecorderHandle picture);

@pragma('wasm:import', 'skwasm.picture_dispose')
external PictureHandle picture_dispose(PictureHandle handle);

@pragma('wasm:import', 'skwasm.picture_approximateBytesUsed')
external RawSize picture_approxmateBytesUsed(PictureHandle handle);
