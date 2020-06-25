// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.mutatorsstack;

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

  public void push(FlutterMutator mutator) {
    mutators.add(mutator);
  }

  List<FlutterMutator> getMutators() {
    return mutators;
  }
}
