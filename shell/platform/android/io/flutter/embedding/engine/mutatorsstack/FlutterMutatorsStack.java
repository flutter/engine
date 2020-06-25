// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.mutatorsstack;

import android.graphics.Matrix;
import android.graphics.Rect;
import android.view.Surface;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

@Keep
public class FlutterMutatorsStack {

  private @NonNull List<FlutterMutator> mutators;

  public FlutterMutatorsStack() {
    this.mutators = new ArrayList<FlutterMutator>();
  }

  public void pushTransform(float[] values) {
    Matrix matrix = new Matrix();
    matrix.setValues(values);
    float[] matrixValues = new float[9];
    matrix.getValues(matrixValues);
    io.flutter.Log.e("matrix ", "----------------- Java matrix -----------------");
    io.flutter.Log.e("matrix ", "MTRANS_X " + matrixValues[2]);
    io.flutter.Log.e("matrix ", "MTRANS_Y " + matrixValues[5]);
    io.flutter.Log.e("matrix ", "MSKEW_X " + matrixValues[1]);
    io.flutter.Log.e("matrix ", "MSKEW_Y " + matrixValues[3]);
    io.flutter.Log.e("matrix ", "MSCALE_X " + matrixValues[0]);
    io.flutter.Log.e("matrix ", "MSCALE_Y " + matrixValues[4]);
    io.flutter.Log.e("matrix ", "MPERSP_X " + matrixValues[7]);
    io.flutter.Log.e("matrix ", "MPERSP_Y " + matrixValues[8]);
    io.flutter.Log.e("matrix ", "MPERSP_Z " + matrixValues[6]);
    FlutterMutator mutator = new FlutterMutator(matrix);
    mutators.add(mutator);
  }

  public List<FlutterMutator> getMutators() {
    return mutators;
  }
}
