// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.animation.Animator;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;

/**
 * {@link SplashScreen} that displays a given {@link Drawable}, which then cross-fades to
 * Flutter's content.
 */
public class DrawableSplashScreen implements SplashScreen {
  private final Drawable drawable;
  private final long crossfadeDurationInMillis;
  private DrawableSplashScreenView splashView;

  /**
   * Constructs a {@code DrawableSplashScreen} that displays the given {@code drawable} and
   * crossfades to Flutter content in 500 milliseconds.
   */
  public DrawableSplashScreen(@NonNull Drawable drawable) {
    this(drawable, 500);
  }

  /**
   * Constructs a {@code DrawableSplashScreen} that displays the given {@code drawable} and
   * crossfades to Flutter content in the given {@code crossfadeDurationInMillis}.
   */
  public DrawableSplashScreen(@NonNull Drawable drawable, long crossfadeDurationInMillis) {
    this.drawable = drawable;
    this.crossfadeDurationInMillis = crossfadeDurationInMillis;
  }

  @Nullable
  @Override
  public View createSplashView(@NonNull Context context) {
    if (splashView == null) {
      splashView = new DrawableSplashScreenView(context);
      splashView.setSplashDrawable(drawable);
    }
    return splashView;
  }

  @Override
  public void transitionToFlutter(@NonNull Runnable onTransitionComplete) {
    splashView.animate()
        .alpha(0.0f)
        .setDuration(crossfadeDurationInMillis)
        .setListener(new Animator.AnimatorListener() {
          @Override
          public void onAnimationStart(Animator animation) {}

          @Override
          public void onAnimationEnd(Animator animation) {
            onTransitionComplete.run();
          }

          @Override
          public void onAnimationCancel(Animator animation) {
            onTransitionComplete.run();
          }

          @Override
          public void onAnimationRepeat(Animator animation) {}
        }
    );
  }

  public static class DrawableSplashScreenView extends FrameLayout {
    private final ImageView imageView;

    public DrawableSplashScreenView(@NonNull Context context) {
      this(context, null, 0);
    }

    public DrawableSplashScreenView(@NonNull Context context, @Nullable AttributeSet attrs) {
      this(context, attrs, 0);
    }

    public DrawableSplashScreenView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
      super(context, attrs, defStyleAttr);

      imageView = new ImageView(context);
      addView(imageView);
    }

    public void setSplashDrawable(@Nullable Drawable drawable) {
      setSplashDrawable(drawable, ImageView.ScaleType.FIT_XY);
    }

    public void setSplashDrawable(@Nullable Drawable drawable, ImageView.ScaleType scaleType) {
      imageView.setScaleType(scaleType);
      imageView.setImageDrawable(drawable);
    }
  }
}
