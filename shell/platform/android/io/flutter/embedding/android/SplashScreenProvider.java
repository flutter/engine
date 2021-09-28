// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import androidx.annotation.Nullable;

/**
 * Provides a {@link SplashScreen} to display while Flutter initializes and renders its first frame.
 *
 * <p>Please use the new Splash screen API available on Android S. On lower versions of Android,
 * it's no longer necessary to display a splash screen to wait for the Flutter first frame, and
 * thus, there is no longer a need to provide an implementation of this interface.
 *
 * @deprecated
 */
@Deprecated
public interface SplashScreenProvider {
  /**
   * Provides a {@link SplashScreen} to display while Flutter initializes and renders its first
   * frame.
   *
   * @return The splash screen.
   */
  @Nullable
  SplashScreen provideSplashScreen();
}
