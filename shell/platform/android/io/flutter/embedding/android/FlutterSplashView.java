// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.view.ViewCompat;
import android.view.View;
import android.widget.FrameLayout;

import io.flutter.app.BuildConfig;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;

public class FlutterSplashView extends FrameLayout {
  private static String TAG = "FlutterSplashView";

  @Nullable
  private final SplashScreen splashScreen;
  @Nullable
  private FlutterView flutterView;
  @Nullable
  private View splashScreenView;

  @NonNull
  private final OnFirstFrameRenderedListener onFirstFrameRenderedListener = new OnFirstFrameRenderedListener() {
    @Override
    public void onFirstFrameRendered() {
      if (splashScreen != null) {
        splashScreen.transitionToFlutter(onTransitionComplete);
      }
    }
  };

  @NonNull
  private final Runnable onTransitionComplete = new Runnable() {
    @Override
    public void run() {
      removeView(splashScreenView);
    }
  };

  public FlutterSplashView(
      @NonNull Context context,
      @Nullable SplashScreen splashScreen
  ) {
    super(context);
    this.splashScreen = splashScreen;
  }

  /**
   * Displays the given {@link FlutterView} in this {@code FlutterSplashView}, or removes
   * an existing {@link FlutterView} if {@code flutterView} is {@code null}.
   */
  public void setFlutterView(@Nullable FlutterView flutterView) {
    if (this.flutterView != null) {
      removeView(this.flutterView);
      removeSplashScreen();
    }

    this.flutterView = flutterView;
    addView(flutterView);

    if (isAttachedToWindow()) {
      showSplashScreen();
    }
  }

  /**
   * Overridden implementation of {@code isAttachedToWindow()} to deal with
   * earlier APIs that don't have this method. {@code ViewCompat} is used
   * for earlier APIs.
   */
  public boolean isAttachedToWindow() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
      return super.isAttachedToWindow();
    } else {
      return ViewCompat.isAttachedToWindow(this);
    }
  }

  @Override
  protected void onAttachedToWindow() {
    super.onAttachedToWindow();
    showSplashScreen();
  }

  @Override
  protected void onDetachedFromWindow() {
    removeSplashScreen();
    super.onDetachedFromWindow();
  }

  /**
   * Shows our splash screen if {@code splashScreen} exists, and if a {@link FlutterView}
   * has been set and that {@link FlutterView} has yet to render its first Flutter frame.
   */
  private void showSplashScreen() {
    // If we were given a splash screen to display, and if Flutter's first frame
    // has yet to be rendered, then show the splash screen.
    if (splashScreen != null && flutterView != null && !flutterView.hasRenderedFirstFrame()) {
      splashScreenView = splashScreen.createSplashView(getContext());
      addView(splashScreenView, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
      flutterView.addOnFirstFrameRenderedListener(onFirstFrameRenderedListener);
    }
  }

  /**
   * Removes the splash screen from the view hierarchy, if it exists, and unregisters
   * our {@code OnFirstFrameRenderedListener}, if it is registered.
   */
  private void removeSplashScreen() {
    if (splashScreenView != null) {
      removeView(splashScreenView);
    }

    if (flutterView != null) {
      flutterView.removeOnFirstFrameRenderedListener(onFirstFrameRenderedListener);
    }
  }
}
