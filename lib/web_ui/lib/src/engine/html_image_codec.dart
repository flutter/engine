// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import 'browser_detection.dart';
import 'dom.dart';
import 'safe_browser_api.dart';
import 'util.dart';

Object? get _jsImageDecodeFunction => getJsProperty<Object?>(
  getJsProperty<Object>(
    getJsProperty<Object>(domWindow, 'Image'),
    'prototype',
  ),
  'decode',
);
final bool _supportsDecode = _jsImageDecodeFunction != null;

typedef WebOnlyImageCodecChunkCallback = void Function(
    int cumulativeBytesLoaded, int expectedTotalBytes);

class HtmlCodec implements ui.Codec {
  HtmlCodec(this.src, {this.chunkCallback});

  final String src;
  final WebOnlyImageCodecChunkCallback? chunkCallback;

  @override
  int get frameCount => 1;

  @override
  int get repetitionCount => 0;

  @override
  Future<ui.FrameInfo> getNextFrame() async {
    final Completer<ui.FrameInfo> completer = Completer<ui.FrameInfo>();
    // Currently there is no way to watch decode progress, so
    // we add 0/100 , 100/100 progress callbacks to enable loading progress
    // builders to create UI.
      chunkCallback?.call(0, 100);
    if (_supportsDecode) {
      final DomHTMLImageElement imgElement = createDomHTMLImageElement();
      imgElement.src = src;
      setJsProperty<String>(imgElement, 'decoding', 'async');

      // Ignoring the returned future on purpose because we're communicating
      // through the `completer`.
      // ignore: unawaited_futures
      imgElement.decode().then((dynamic _) {
        chunkCallback?.call(100, 100);
        int naturalWidth = imgElement.naturalWidth.toInt();
        int naturalHeight = imgElement.naturalHeight.toInt();
        // Workaround for https://bugzilla.mozilla.org/show_bug.cgi?id=700533.
        if (naturalWidth == 0 && naturalHeight == 0 && browserEngine == BrowserEngine.firefox) {
          const int kDefaultImageSizeFallback = 300;
          naturalWidth = kDefaultImageSizeFallback;
          naturalHeight = kDefaultImageSizeFallback;
        }
        final HtmlImage image = HtmlImage(
          imgElement,
          naturalWidth,
          naturalHeight,
        );
        completer.complete(SingleFrameInfo(image));
      }).catchError((dynamic e) {
        // This code path is hit on Chrome 80.0.3987.16 when too many
        // images are on the page (~1000).
        // Fallback here is to load using onLoad instead.
        _decodeUsingOnLoad(completer);
      });
    } else {
      _decodeUsingOnLoad(completer);
    }
    return completer.future;
  }

  void _decodeUsingOnLoad(Completer<ui.FrameInfo> completer) {
    final DomHTMLImageElement imgElement = createDomHTMLImageElement();
    // If the browser doesn't support asynchronous decoding of an image,
    // then use the `onload` event to decide when it's ready to paint to the
    // DOM. Unfortunately, this will cause the image to be decoded synchronously
    // on the main thread, and may cause dropped framed.
    late DomEventListener errorListener;
    DomEventListener? loadListener;
    errorListener = allowInterop((DomEvent event) {
      if (loadListener != null) {
        imgElement.removeEventListener('load', loadListener);
      }
      imgElement.removeEventListener('error', errorListener);
      completer.completeError(event);
    });
    imgElement.addEventListener('error', errorListener);
    loadListener = allowInterop((DomEvent event) {
      if (chunkCallback != null) {
        chunkCallback!(100, 100);
      }
      imgElement.removeEventListener('load', loadListener);
      imgElement.removeEventListener('error', errorListener);
      final HtmlImage image = HtmlImage(
        imgElement,
        imgElement.naturalWidth.toInt(),
        imgElement.naturalHeight.toInt(),
      );
      completer.complete(SingleFrameInfo(image));
    });
    imgElement.addEventListener('load', loadListener);
    imgElement.src = src;
  }

  @override
  void dispose() {}
}

class HtmlBlobCodec extends HtmlCodec {
  HtmlBlobCodec(this.blob) : super(domWindow.URL.createObjectURL(blob));

  final DomBlob blob;

  @override
  void dispose() {
    domWindow.URL.revokeObjectURL(src);
  }
}

class SingleFrameInfo implements ui.FrameInfo {
  SingleFrameInfo(this.image);

  @override
  Duration get duration => Duration.zero;

  @override
  final ui.Image image;
}

class HtmlImage implements ui.Image {
  HtmlImage(this.imgElement, this.width, this.height) {
    ui.Image.onCreate?.call(this);
  }

  final DomHTMLImageElement imgElement;
  bool _didClone = false;

  bool _disposed = false;
  @override
  void dispose() {
    ui.Image.onDispose?.call(this);
    // Do nothing. The codec that owns this image should take care of
    // releasing the object url.
    if (assertionsEnabled) {
      _disposed = true;
    }
  }

  @override
  bool get debugDisposed {
    if (assertionsEnabled) {
      return _disposed;
    }
    return throw StateError('Image.debugDisposed is only available when asserts are enabled.');
  }


  @override
  ui.Image clone() => this;

  @override
  bool isCloneOf(ui.Image other) => other == this;

  @override
  List<StackTrace>? debugGetOpenHandleStackTraces() => null;

  @override
  final int width;

  @override
  final int height;

  @override
  Future<ByteData?> toByteData({ui.ImageByteFormat format = ui.ImageByteFormat.rawRgba}) {
    switch (format) {
      // Solution based on https://stackoverflow.com/a/60564905/4609658
      case ui.ImageByteFormat.rawRgba:
        final ByteBuffer buffer = _getStraightRgba();
        _straightRgbaToPremultipliedRgba(buffer);
        return Future<ByteData?>.value(buffer.asByteData());
      case ui.ImageByteFormat.rawStraightRgba:
        final ByteBuffer buffer = _getStraightRgba();
        return Future<ByteData?>.value(buffer.asByteData());
      default:
        if (imgElement.src?.startsWith('data:') ?? false) {
          final UriData data = UriData.fromUri(Uri.parse(imgElement.src!));
          return Future<ByteData?>.value(data.contentAsBytes().buffer.asByteData());
        } else {
          return Future<ByteData?>.value();
        }
    }
  }

  /// Converts [imgElement] to straight RGBA.
  /// Prefer [WebGLContext] whenever possible since Canvas keeps the RGBA as premultiplied-alpha
  /// and [DomCanvasRenderingContext2DExtension.getImageData] converts back to straight RGBA
  /// which is lossy algorithm for instance when Alpha chanel is 0
  /// then there is no way to get back original straight RGBA
  /// https://stackoverflow.com/questions/23497925/how-can-i-stop-the-alpha-premultiplication-with-canvas-imagedata
  ByteBuffer _getStraightRgba() {
    if (webGLVersion >= 2) {
      final DomCanvasElement canvas = createDomCanvasElement()
        ..width = width.toDouble()
        ..height = height.toDouble();
      final WebGLContext gl = canvas.getGlContext(webGLVersion);
      gl.activeTexture(gl.texture0);
      final WebGLTexture? texture = gl.createTexture();
      gl.bindTexture(gl.texture2d, texture);
      final WebGLFramebuffer? framebuffer = gl.createFramebuffer();
      gl.bindFramebuffer(gl.framebuffer, framebuffer);
      gl.framebufferTexture2D(gl.framebuffer, gl.colorAttachment0, gl.texture2d, texture, 0);
      gl.texImage2D(gl.texture2d, 0, gl.rgba, gl.rgba, gl.unsignedByte, imgElement);
      gl.drawBuffers(<int>[gl.colorAttachment0]);
      final Uint8List data = Uint8List(width * height * 4);
      gl.readPixels(0, 0, width.toDouble(), height.toDouble(), gl.rgba, gl.unsignedByte, data);
      return data.buffer;
    } else {
      final DomCanvasElement canvas = createDomCanvasElement()
        ..width = width.toDouble()
        ..height = height.toDouble();
      final DomCanvasRenderingContext2D ctx = canvas.context2D;
      ctx.drawImage(imgElement, 0, 0);
      final DomImageData imageData = ctx.getImageData(0, 0, width, height);
      return imageData.data.buffer;
    }
  }

  /// Mutates the [pixels], converting them from straight RGBA to premultiplied RGBA.
  void _straightRgbaToPremultipliedRgba(ByteBuffer pixels) {
    final int pixelCount = pixels.lengthInBytes ~/ 4;
    final Uint8List pixelBytes = pixels.asUint8List();
    for (int i = 0; i < pixelCount; i += 4) {
      final double alpha = pixelBytes[i + 3] / 255;
      pixelBytes[i] = (pixelBytes[i] * alpha).floor();
      pixelBytes[i + 1] = (pixelBytes[i + 1] * alpha).floor();
      pixelBytes[i + 2] = (pixelBytes[i + 2] * alpha).floor();
    }
  }

  DomHTMLImageElement cloneImageElement() {
    if (!_didClone) {
      _didClone = true;
      imgElement.style.position = 'absolute';
    }
    return imgElement.cloneNode(true) as DomHTMLImageElement;
  }

  @override
  String toString() => '[$width\u00D7$height]';
}
