// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import static io.flutter.embedding.engine.systemchannels.PlatformViewsChannel.PlatformViewBufferSize;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.BlendMode;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import io.flutter.Log;
import io.flutter.embedding.android.AndroidTouchProcessor;
import io.flutter.util.ViewUtils;

@TargetApi(Build.VERSION_CODES.M)
class PlatformViewWrapper extends FrameLayout {
  private static final String TAG = "PlatformViewWrapper";

  private int prevLeft;
  private int prevTop;
  private int left;
  private int top;
  private SurfaceTexture tx;
  private Surface surface;
  private AndroidTouchProcessor touchProcessor;

  private @Nullable PlatformViewBufferSize bufferSize;
  @Nullable @VisibleForTesting ViewTreeObserver.OnGlobalFocusChangeListener activeFocusListener;

  public PlatformViewWrapper(@NonNull Context context) {
    super(context);
    setWillNotDraw(false);
  }

  public void setTouchProcessor(@Nullable AndroidTouchProcessor touchProcessor) {
    this.touchProcessor = touchProcessor;
  }

  public void setTexture(@Nullable SurfaceTexture tx) {
    if (tx != null) {
      tx.release();
    }
    this.tx = tx;

    if (surface != null) {
      surface.release();
    }
    surface = new Surface(tx);

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      // Fill the entire canvas with a transparent color.
      // As a result, the background color of the platform view container is displayed
      // to the user until the platform view draws its first frame.
      final Canvas canvas = surface.lockHardwareCanvas();
      canvas.drawColor(Color.TRANSPARENT);
      surface.unlockCanvasAndPost(canvas);
    } else {
      Log.e(TAG, "Platform views cannot be displayed below API level 23.");
    }
  }

  public void setLayoutParams(@NonNull FrameLayout.LayoutParams params) {
    super.setLayoutParams(params);

    left = params.leftMargin;
    top = params.topMargin;
  }

  public void setBufferSize(int width, int height) {
    bufferSize = new PlatformViewBufferSize(width, height);
    if (tx != null) {
      tx.setDefaultBufferSize(width, height);
    }
  }

  /** Returns the size of the buffer where the platform view pixels are written to. */
  @Nullable
  public PlatformViewBufferSize getBufferSize() {
    return bufferSize;
  }

  @Nullable
  public SurfaceTexture getTexture() {
    return tx;
  }

  public void release() {
    if (tx != null) {
      tx.release();
      tx = null;
    }
    if (surface != null) {
      surface.release();
      surface = null;
    }
  }

  @Override
  public boolean onInterceptTouchEvent(@NonNull MotionEvent event) {
    return true;
  }

  @Override
  public void onDescendantInvalidated(@NonNull View child, @NonNull View target) {
    invalidate();
  }

  @Override
  public void draw(Canvas canvas) {
    if (surface == null || !surface.isValid()) {
      Log.e(TAG, "Invalid surface. The platform view cannot be displayed.");
      return;
    }
    if (tx == null || tx.isReleased()) {
      Log.e(TAG, "Invalid texture. The platform view cannot be displayed.");
      return;
    }
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
      Log.e(TAG, "Platform views cannot be displayed below API level 23.");
      return;
    }
    // Override the canvas that this subtree of views will use to draw.
    final Canvas surfaceCanvas = surface.lockHardwareCanvas();
    try {
      // Clear the current pixels in the canvas.
      // This helps when a WebView renders an HTML document with transparent background.
      surfaceCanvas.drawColor(Color.TRANSPARENT, BlendMode.CLEAR);
      super.draw(surfaceCanvas);
    } finally {
      surface.unlockCanvasAndPost(surfaceCanvas);
    }
  }

  @Override
  @SuppressLint("ClickableViewAccessibility")
  public boolean onTouchEvent(@NonNull MotionEvent event) {
    if (touchProcessor == null) {
      return super.onTouchEvent(event);
    }
    final Matrix screenMatrix = new Matrix();
    switch (event.getAction()) {
      case MotionEvent.ACTION_DOWN:
        prevLeft = left;
        prevTop = top;
        screenMatrix.postTranslate(left, top);
        break;
      case MotionEvent.ACTION_MOVE:
        // While the view is dragged, use the left and top positions as
        // they were at the moment the touch event fired.
        screenMatrix.postTranslate(prevLeft, prevTop);
        prevLeft = left;
        prevTop = top;
        break;
      case MotionEvent.ACTION_UP:
      default:
        screenMatrix.postTranslate(left, top);
        break;
    }
    return touchProcessor.onTouchEvent(event, screenMatrix);
  }

  public void setOnDescendantFocusChangeListener(@NonNull OnFocusChangeListener userFocusListener) {
    unsetOnDescendantFocusChangeListener();
    final ViewTreeObserver observer = getViewTreeObserver();
    if (observer.isAlive() && activeFocusListener == null) {
      activeFocusListener =
          new ViewTreeObserver.OnGlobalFocusChangeListener() {
            @Override
            public void onGlobalFocusChanged(View oldFocus, View newFocus) {
              userFocusListener.onFocusChange(
                  PlatformViewWrapper.this, ViewUtils.childHasFocus(PlatformViewWrapper.this));
            }
          };
      observer.addOnGlobalFocusChangeListener(activeFocusListener);
    }
  }

  public void unsetOnDescendantFocusChangeListener() {
    final ViewTreeObserver observer = getViewTreeObserver();
    if (observer.isAlive() && activeFocusListener != null) {
      final ViewTreeObserver.OnGlobalFocusChangeListener currFocusListener = activeFocusListener;
      activeFocusListener = null;
      observer.removeOnGlobalFocusChangeListener(currFocusListener);
    }
  }
}
