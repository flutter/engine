// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.mutatorsstack;

import android.view.Surface;
import android.graphics.Matrix;
import android.graphics.Rect;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

@Keep
public class FlutterMutator {

  @Nullable private Matrix matrix;
  @Nullable private Rect rect;
  private FlutterMutatorType type;

  public FlutterMutator(Rect rect) {
    this.type = FlutterMutatorType.CLIP_RECT;
    this.rect = rect;
  }

  public FlutterMutator(Matrix matrix) {
    this.type = FlutterMutatorType.TRANSFORM;
    this.matrix = matrix;
  }

  public FlutterMutatorType getType() {
      return type;
  }

  public Rect getRect() {
    return rect;
  }

  public Matrix getMatrix() {
    return matrix;
  }
}
