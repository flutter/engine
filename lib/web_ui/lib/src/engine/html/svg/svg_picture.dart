// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../../dom.dart';
import '../../picture.dart';
import '../surface.dart';
import 'svg_canvas.dart';

const bool kUseSvgPicture = true;

class SvgPicture extends PersistedLeafSurface {
  SvgPicture(this.dx, this.dy, this.picture, this.hints)
      : localPaintBounds = picture.recordingCanvas!.pictureBounds,
        _id = _pictureCounter++;

  static int _pictureCounter = 1;

  final int _id;
  final double dx;
  final double dy;
  final EnginePicture picture;
  final ui.Rect? localPaintBounds;
  final int hints;

  SvgCanvas? _canvas;

  @override
  void adoptElements(SvgPicture oldSurface) {
    super.adoptElements(oldSurface);
    _canvas = oldSurface._canvas;
  }

  @override
  DomElement createElement() {
    final DomElement element = defaultCreateElement('flt-svg-picture');
    element.setAttribute('flt-id', _id);

    // The DOM elements used to render pictures are used purely to put pixels on
    // the screen. They have no semantic information. If an assistive technology
    // attempts to scan picture content it will look like garbage and confuse
    // users. UI semantics are exported as a separate DOM tree rendered parallel
    // to pictures.
    //
    // Why are layer and scene elements not hidden from ARIA? Because those
    // elements may contain platform views, and platform views must be
    // accessible.
    element.setAttribute('aria-hidden', 'true');

    return element;
  }

  @override
  void preroll(PrerollSurfaceContext prerollContext) {
    if (prerollContext.activeShaderMaskCount != 0 ||
        prerollContext.activeColorFilterCount != 0) {
      picture.recordingCanvas?.renderStrategy.isInsideSvgFilterTree = true;
    }
    super.preroll(prerollContext);
  }

  @override
  void recomputeTransformAndClip() {
    transform = parent!.transform;
    if (dx != 0.0 || dy != 0.0) {
      transform = transform!.clone();
      transform!.translate2D(dx, dy);
    }
  }

  @override
  double matchForUpdate(SvgPicture existingSurface) {
    if (existingSurface.picture == picture) {
      // Picture is the same, return perfect score.
      return 0.0;
    }
    return 1.0;
  }

  @override
  void apply() {
    _applyTranslate();
    _redraw();
  }

  @override
  void update(SvgPicture oldSurface) {
    super.update(oldSurface);

    if (dx != oldSurface.dx || dy != oldSurface.dy) {
      _applyTranslate();
    }

    if (identical(picture, oldSurface.picture)) {
      _canvas = oldSurface._canvas;
    } else {
      _redraw();
    }
  }

  void _applyTranslate() {
    rootElement!.style.transform = 'translate(${dx}px, ${dy}px)';
  }

  void _redraw() {
    _canvas?.discard();
    final SvgCanvas canvas = SvgCanvas();
    _canvas = canvas;
    canvas.draw(picture);
    rootElement!.append(canvas.rootElement);
  }

  @override
  void discard() {
    super.discard();
    _canvas?.discard();
    _canvas = null;
  }
}
