// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.mutatorsstack;

import android.graphics.Matrix;
import android.graphics.Path;
import android.graphics.Rect;
import android.graphics.RectF;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import java.util.ArrayList;
import java.util.List;

@Keep
public class FlutterMutatorsStack {

  private @NonNull List<FlutterMutator> mutators;

  private List<Path> finalClippingPaths;
  private Matrix finalMatrix;

  public FlutterMutatorsStack() {
    this.mutators = new ArrayList<FlutterMutator>();
    finalMatrix = new Matrix();
    finalClippingPaths = new ArrayList<Path>();
  }

  public void pushTransform(float[] values) {
    Matrix matrix = new Matrix();
    matrix.setValues(values);
    FlutterMutator mutator = new FlutterMutator(matrix);
    mutators.add(mutator);
    finalMatrix.preConcat(mutator.getMatrix());
  }

  public void pushClipRect(int left, int top, int right, int bottom) {
    Rect rect = new Rect(left, top, right, bottom);
    FlutterMutator mutator = new FlutterMutator(rect);
    mutators.add(mutator);
    Path path = new Path();
    path.addRect(new RectF(rect), Path.Direction.CCW);
    path.transform(finalMatrix);
    finalClippingPaths.add(path);
  }

  public List<FlutterMutator> getMutators() {
    return mutators;
  }

  public List<Path> getFinalClippingPaths() {
    return finalClippingPaths;
  }

  public Matrix getFinalMatrix() {
    return finalMatrix;
  }
}
