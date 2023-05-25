// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmPicture implements SkwasmObjectWrapper<RawPicture>, ScenePicture {
  SkwasmPicture.fromHandle(this.handle) {
    _registry.register(this);
  }

  static final SkwasmFinalizationRegistry<RawPicture> _registry =
    SkwasmFinalizationRegistry<RawPicture>(pictureDispose);

  @override
  final PictureHandle handle;
  bool _isDisposed = false;

  @override
  void dispose() {
    assert(!_isDisposed);
    ui.Picture.onDispose?.call(this);
    _registry.unregister(this);
    pictureDispose(handle);
    _isDisposed = true;
  }

  @override
  Future<ui.Image> toImage(int width, int height) async => toImageSync(width, height);


  @override
  int get approximateBytesUsed => pictureApproximateBytesUsed(handle);

  @override
  bool debugDisposed = false;

  @override
  ui.Image toImageSync(int width, int height) =>
    SkwasmImage(imageCreateFromPicture(handle, width, height));

  @override
  ui.Rect get cullRect {
    return withStackScope((StackScope s) {
      final RawRect rect = s.allocFloatArray(4);
      pictureGetCullRect(handle, rect);
      return s.convertRectFromNative(rect);
    });
  }
}

class SkwasmPictureRecorder implements SkwasmObjectWrapper<RawPictureRecorder>, ui.PictureRecorder {
  factory SkwasmPictureRecorder() =>
    SkwasmPictureRecorder._fromHandle(pictureRecorderCreate());

  SkwasmPictureRecorder._fromHandle(this.handle) {
    _registry.register(this);
  }


  static final SkwasmFinalizationRegistry<RawPictureRecorder> _registry =
    SkwasmFinalizationRegistry<RawPictureRecorder>(pictureRecorderDispose);

  @override
  final PictureRecorderHandle handle;
  bool _isDisposed = false;

  void dispose() {
    assert(!_isDisposed);
    _registry.unregister(this);
    pictureRecorderDispose(handle);
    _isDisposed = true;
  }

  @override
  SkwasmPicture endRecording() {
    isRecording = false;

    final SkwasmPicture picture = SkwasmPicture.fromHandle(
      pictureRecorderEndRecording(handle)
    );
    ui.Picture.onCreate?.call(picture);
    return picture;
  }

  @override
  bool isRecording = true;
}
