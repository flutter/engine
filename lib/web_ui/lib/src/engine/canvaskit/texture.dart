// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../texture.dart';
import 'canvaskit_api.dart';
import 'image.dart';
import 'layer.dart';
import 'painting.dart';

class CkTexture {
  CkTexture(this.source);

  final Object source;

  CkImage? _ckImage;

  bool _newFrameReady = false;

  final CkPaint _paint = CkPaint();

  void markNewFrameAvailable() {
    _newFrameReady = true;
  }

  void paint(
    PaintContext context,
    ui.Offset offset,
    double width,
    double height,
    bool freeze,
    ui.FilterQuality filterQuality,
  ) {
    if (_ckImage == null || _newFrameReady && !freeze) {
      final SkImage? skImage =
          canvasKit.MakeLazyImageFromTextureSource(source, null);

      if (skImage == null) {
        return;
      }

      _ckImage?.dispose();
      _ckImage = CkImage(skImage);
    }

    context.leafNodesCanvas?.drawImageRect(
      _ckImage!,
      ui.Rect.fromLTWH(
        0,
        0,
        _ckImage!.width.toDouble(),
        _ckImage!.height.toDouble(),
      ),
      ui.Rect.fromLTWH(offset.dx, offset.dy, width, height),
      _paint..filterQuality = filterQuality,
    );
  }

  void dispose() {
    _ckImage?.dispose();
  }
}

class CkTextureRegistry implements TextureRegistry {
  CkTextureRegistry._();

  static final CkTextureRegistry instance = CkTextureRegistry._();

  final Map<int, CkTexture> _textures = <int, CkTexture>{};

  int _nextTextureId = 0;

  @override
  int registerTexture(Object source) {
    final int id = _nextTextureId++;
    _textures[id] = CkTexture(source);
    return id;
  }

  @override
  void unregisterTexture(int id) {
    final CkTexture? texture = _textures.remove(id);
    texture?.dispose();
  }

  CkTexture? getTexture(int id) {
    return _textures[id];
  }

  @override
  void textureFrameAvailable(int id) {
    final CkTexture? texture = _textures[id];
    if (texture != null) {
      texture.markNewFrameAvailable();
    }
  }
}
