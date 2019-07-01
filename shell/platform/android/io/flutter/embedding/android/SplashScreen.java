// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.View;

/**
 * Splash screen configuration for a given Flutter experience.
 * <p>
 * Implementations provides a visual representation of a splash screen in
 * {@link #createSplashView(Context)}, and implement a transition from the
 * splash UI to Flutter's UI in {@link #transitionToFlutter(Runnable)}.
 */
public interface SplashScreen {
  /**
   * Creates a {@code View} to be displayed as a splash screen before
   * Flutter renders its first frame.
   */
  @Nullable
  View createSplashView(@NonNull Context context);

  /**
   * Invoked by Flutter when Flutter has rendered its first frame, and would like
   * the splash {@code View} to disappear.
   * <p>
   * The provided {@code onTransitionComplete} callback must be invoked when
   * the splash {@code View} has finished transitioning itself away. The splash
   * {@code View} will be removed and destroyed when the callback is invoked.
   */
  void transitionToFlutter(@NonNull Runnable onTransitionComplete);
}
