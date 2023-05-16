// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import android.content.Context;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import androidx.annotation.NonNull;

/** Android implementation of the platform plugin. */
public final class SynchronousPlatformPlugin {
  @NonNull private final Context context;

  public SynchronousPlatformPlugin(Context context) {
    this.context = context;
  }

  public Float getScaledFontSize(float fontSize) {
    final DisplayMetrics metrics = context.getResources().getDisplayMetrics();
    return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, fontSize, metrics)
        / TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 1.0f, metrics);
  }
}
