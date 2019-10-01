// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.lifecycle;

import android.arch.lifecycle.Lifecycle;
import android.support.annotation.NonNull;

public class ConcreteLifecycleReference implements LifecycleReference {
  @NonNull
  private final Lifecycle lifecycle;

  public ConcreteLifecycleReference(@NonNull Lifecycle lifecycle) {
    this.lifecycle = lifecycle;
  }

  @NonNull
  public Lifecycle getLifecycle() {
    return lifecycle;
  }
}
