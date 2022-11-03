import 'package:ui/ui.dart' as ui;

import 'raw/raw_picture.dart';

class SkwasmPicture implements ui.Picture {
  SkwasmPicture.fromHandle(this._handle);
  final PictureHandle _handle;

  PictureHandle get handle => _handle;

  @override
  Future<ui.Image> toImage(int width, int height) {
    throw UnimplementedError();
  }

  @override
  void dispose() => pictureDispose(_handle);

  @override
  int get approximateBytesUsed => pictureApproximateBytesUsed(_handle);
  
  @override
  // TODO(jacksongardner): implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();
  
  @override
  ui.Image toImageSync(int width, int height) {
    // TODO(jacksongardner): implement toImageSync
    throw UnimplementedError();
  }
}

class SkwasmPictureRecorder implements ui.PictureRecorder {
  factory SkwasmPictureRecorder() => 
    SkwasmPictureRecorder._fromHandle(pictureRecorderCreate());

  SkwasmPictureRecorder._fromHandle(this._handle);
  final PictureRecorderHandle _handle;

  PictureRecorderHandle get handle => _handle;

  void delete() => pictureRecorderDestroy(_handle);

  @override
  SkwasmPicture endRecording() => 
    SkwasmPicture.fromHandle(pictureRecorderEndRecording(_handle));

  @override
  // TODO(jacksongardner): implement isRecording
  bool get isRecording => throw UnimplementedError();
}
