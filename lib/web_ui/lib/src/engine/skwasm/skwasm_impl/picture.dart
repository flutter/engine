// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/ui.dart' as ui;

class SkwasmPicture implements ScenePicture {
  SkwasmPicture.fromHandle(this._handle) {
    _registry.register(this, handle.address, this);
  }

  static final DomFinalizationRegistry _registry =
    DomFinalizationRegistry(createSkwasmFinalizer(pictureDispose));

  final PictureHandle _handle;
  bool _isDisposed = false;

  @override
  void dispose() {
    assert(!_isDisposed);
    ui.Picture.onDispose?.call(this);
    _registry.unregister(this);
    pictureDispose(_handle);
    _isDisposed = true;
  }

  PictureHandle get handle => _handle;

  @override
  Future<ui.Image> toImage(int width, int height) async => toImageSync(width, height);


  @override
  int get approximateBytesUsed => pictureApproximateBytesUsed(_handle);

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

class SkwasmPictureRecorder implements ui.PictureRecorder {
  factory SkwasmPictureRecorder() =>
    SkwasmPictureRecorder._fromHandle(pictureRecorderCreate());

  SkwasmPictureRecorder._fromHandle(this._handle) {
    _registry.register(this, _handle.address, this);
  }


  static final DomFinalizationRegistry _registry =
    DomFinalizationRegistry(createSkwasmFinalizer(pictureRecorderDispose));

  final PictureRecorderHandle _handle;
  bool _isDisposed = false;

  PictureRecorderHandle get handle => _handle;

  void dispose() {
    assert(!_isDisposed);
    _registry.unregister(this);
    pictureRecorderDispose(_handle);
    _isDisposed = true;
  }

  @override
  SkwasmPicture endRecording() {
    isRecording = false;

    final SkwasmPicture picture = SkwasmPicture.fromHandle(pictureRecorderEndRecording(_handle));
    ui.Picture.onCreate?.call(picture);
    return picture;
  }

  @override
  bool isRecording = true;
}
