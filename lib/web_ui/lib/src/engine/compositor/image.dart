// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// Instantiates a [ui.Codec] backed by an `SkImage` from Skia.
void skiaInstantiateImageCodec(Uint8List list, Callback<ui.Codec> callback,
    [int width, int height, int format, int rowBytes]) {
  final js.JsObject skAnimatedImage =
      canvasKit.callMethod('MakeAnimatedImageFromEncoded', <Uint8List>[list]);
  final SkAnimatedImage animatedImage = SkAnimatedImage(skAnimatedImage);
  final SkAnimatedImageCodec codec = SkAnimatedImageCodec(animatedImage);
  callback(codec);
}

/// A wrapper for `SkAnimatedImage`.
class SkAnimatedImage implements ui.Image {
  final js.JsObject _skAnimatedImage;

  SkAnimatedImage(this._skAnimatedImage);

  @override
  void dispose() {
    _skAnimatedImage.callMethod('delete');
  }

  int get frameCount => _skAnimatedImage.callMethod('getFrameCount');

  /// Decodes the next frame and returns the frame duration.
  int decodeNextFrame() => _skAnimatedImage.callMethod('decodeNextFrame');

  int get repetitionCount => _skAnimatedImage.callMethod('getRepetitionCount');

  SkImage get currentFrameAsImage {
    final js.JsObject _currentFrame =
        _skAnimatedImage.callMethod('getCurrentFrame');
    return SkImage(_currentFrame);
  }

  @override
  int get width => _skAnimatedImage.callMethod('width');

  @override
  int get height => _skAnimatedImage.callMethod('height');

  @override
  Future<ByteData> toByteData(
      {ui.ImageByteFormat format = ui.ImageByteFormat.rawRgba}) {
    throw 'unimplemented';
  }
}

/// A [ui.Image] backed by an `SkImage` from Skia.
class SkImage implements ui.Image {
  js.JsObject skImage;

  SkImage(this.skImage);

  @override
  void dispose() {
    skImage.callMethod('delete');
    skImage = null;
  }

  @override
  int get width => skImage.callMethod('width');

  @override
  int get height => skImage.callMethod('height');

  @override
  Future<ByteData> toByteData(
      {ui.ImageByteFormat format = ui.ImageByteFormat.rawRgba}) {
    throw 'unimplemented';
  }
}

class SkAnimatedImageCodec implements ui.Codec {
  SkAnimatedImage animatedImage;

  SkAnimatedImageCodec(this.animatedImage);

  @override
  void dispose() {
    animatedImage.dispose();
    animatedImage = null;
  }

  @override
  int get frameCount => animatedImage.frameCount;

  @override
  int get repetitionCount => animatedImage.repetitionCount;

  @override
  Future<ui.FrameInfo> getNextFrame() {
    final int duration = animatedImage.decodeNextFrame();
    final SkImage image = animatedImage.currentFrameAsImage;
    return Future<ui.FrameInfo>.value(AnimatedImageFrameInfo(duration, image));
  }
}

class AnimatedImageFrameInfo implements ui.FrameInfo {
  final Duration _duration;
  final SkImage _image;

  AnimatedImageFrameInfo(int duration, this._image)
      : _duration = Duration(milliseconds: duration);

  @override
  Duration get duration => _duration;

  @override
  ui.Image get image => _image;
}

/// A [ui.Codec] backed by an `SkImage` from Skia.
class SkImageCodec implements ui.Codec {
  final SkImage skImage;

  SkImageCodec(this.skImage);

  @override
  void dispose() {
    // TODO: implement dispose
  }

  @override
  int get frameCount => 1;

  @override
  Future<ui.FrameInfo> getNextFrame() {
    return Future<ui.FrameInfo>.value(SingleFrameInfo(skImage));
  }

  @override
  int get repetitionCount => 0;
}
