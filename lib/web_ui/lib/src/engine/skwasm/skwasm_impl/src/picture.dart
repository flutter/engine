import 'image.dart';
import 'raw/raw_picture.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmPicture implements ui.Picture {
  SkwasmPicture.fromHandle(this._handle);
  final PictureHandle _handle;

  PictureHandle get handle => _handle;

  Future<ui.Image> toImage(int width, int height) {
    throw UnimplementedError();
  }

  void dispose() {
    picture_dispose(_handle);
  }

  int get approximateBytesUsed {
    return picture_approxmateBytesUsed(_handle).toIntSigned();
  }
  
  @override
  // TODO: implement debugDisposed
  bool get debugDisposed => throw UnimplementedError();
  
  @override
  ui.Image toImageSync(int width, int height) {
    // TODO: implement toImageSync
    throw UnimplementedError();
  }
}

class SkwasmPictureRecorder implements ui.PictureRecorder {
  factory SkwasmPictureRecorder() {
    return SkwasmPictureRecorder._fromHandle(pictureRecorder_create());
  }

  SkwasmPictureRecorder._fromHandle(this._handle);
  final PictureRecorderHandle _handle;

  PictureRecorderHandle get handle => _handle;

  void delete() {
    pictureRecorder_destroy(_handle);
  }

  SkwasmPicture endRecording() {
    return SkwasmPicture.fromHandle(pictureRecorder_endRecording(_handle));
  }
}
