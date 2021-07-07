// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;
import 'dart:typed_data';

import 'package:ui/src/engine.dart' show domRenderer, DomRenderer;
import 'package:ui/ui.dart' as ui;

import '../util.dart';
import '../vector_math.dart';
import 'surface.dart';

/// A surface that transforms its children using CSS transform.
class PersistedTransform extends PersistedContainerSurface
    implements ui.TransformEngineLayer {
  PersistedTransform(PersistedTransform? oldLayer, this.matrix4)
      : super(oldLayer);

  final Float32List matrix4;

  @override
  void recomputeTransformAndClip() {
    transform = parent!.transform!.multiplied(Matrix4.fromFloat32List(matrix4));
    localTransformInverse = null;
    projectedClip = null;
  }

  @override
  Matrix4? get defaultLocalTransformInverse =>
      Matrix4.tryInvert(Matrix4.fromFloat32List(matrix4));

  @override
  html.Element createElement() {
    html.Element element = domRenderer.createElement('flt-transform');
    DomRenderer.setElementStyle(element, 'position', 'absolute');
    DomRenderer.setElementStyle(element, 'transform-origin', '0 0 0');
    return element;
  }

  @override
  void apply() {
    rootElement!.style.transform = float64ListToCssTransform(matrix4);
  }

  @override
  void update(PersistedTransform oldSurface) {
    super.update(oldSurface);

    if (identical(oldSurface.matrix4, matrix4)) {
      return;
    }

    bool matrixChanged = false;
    for (int i = 0; i < matrix4.length; i++) {
      if (matrix4[i] != oldSurface.matrix4[i]) {
        matrixChanged = true;
        break;
      }
    }

    if (matrixChanged) {
      apply();
    }
  }
}
