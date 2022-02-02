// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
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

  private int sPrevLeft;
  private int sPrevTop;
  private int sLeft;
  private int sTop;
  private int sWidth;
  private int sHeight;
  private SurfaceTexture sTx;
  private Surface sSurface;
  private AndroidTouchProcessor sTouchProcessor;

  @Nullable @VisibleForTesting ViewTreeObserver.OnGlobalFocusChangeListener activeFocusListener;

  public PlatformViewWrapper(@NonNull Context context) {
    super(context);
    setWillNotDraw(false);
  }

  public void setTouchProcessor(@Nullable AndroidTouchProcessor touchProcessor) {
    sTouchProcessor = touchProcessor;
  }

  public void setTexture(@Nullable SurfaceTexture tx) {
    if (sTx != null) {
      sTx.release();
    }
    sTx = tx;

    if (sSurface != null) {
      sSurface.release();
    }
    sSurface = new Surface(sTx);

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      // Fill the entire canvas with a transparent color.
      // As a result, the background color of the platform view container is displayed
      // to the user
      // until the platform view draws its first frame.
      final Canvas canvas = sSurface.lockHardwareCanvas();
      canvas.drawColor(Color.TRANSPARENT);
      sSurface.unlockCanvasAndPost(canvas);
    } else {
      Log.e(TAG, "Platform views cannot be displayed below API level 23.");
    }
  }

  public void setLayoutParams(@NonNull FrameLayout.LayoutParams params) {
    super.setLayoutParams(params);

    sLeft = params.leftMargin;
    sTop = params.topMargin;
  }

  public void setDefaultBufferSize(int width, int height) {
    sWidth = width;
    sHeight = height;
    if (sTx != null) {
      sTx.setDefaultBufferSize(sWidth, sHeight);
    }
  }

  public int getBufferWidth() {
    return sWidth;
  }

  public int getBufferHeight() {
    return sHeight;
  }

  @Nullable
  public SurfaceTexture getTexture() {
    return sTx;
  }

  public void release() {
    if (sTx != null) {
      sTx.release();
      sTx = null;
    }
    if (sSurface != null) {
      sSurface.release();
      sSurface = null;
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
    if (sSurface == null || !sSurface.isValid()) {
      Log.e(TAG, "Invalid surface. The platform view cannot be displayed.");
      return;
    }
    if (sTx == null || sTx.isReleased()) {
      Log.e(TAG, "Invalid texture. The platform view cannot be displayed.");
      return;
    }
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
      Log.e(TAG, "Platform views cannot be displayed below API level 23.");
      return;
    }
    // Override the canvas that this subtree of views will use to draw.
    final Canvas surfaceCanvas = sSurface.lockHardwareCanvas();
    try {
      super.draw(surfaceCanvas);
    } finally {
      sSurface.unlockCanvasAndPost(surfaceCanvas);
    }
  }

  @Override
  @SuppressLint("ClickableViewAccessibility")
  public boolean onTouchEvent(@NonNull MotionEvent event) {
    if (sTouchProcessor == null) {
      return super.onTouchEvent(event);
    }
    final Matrix screenMatrix = new Matrix();
    switch (event.getAction()) {
      case MotionEvent.ACTION_DOWN:
        sPrevLeft = sLeft;
        sPrevTop = sTop;
        screenMatrix.postTranslate(sLeft, sTop);
        break;
      case MotionEvent.ACTION_MOVE:
        // While the view is dragged, use the left and top positions as
        // they were at the moment the touch event fired.
        screenMatrix.postTranslate(sPrevLeft, sPrevTop);
        sPrevLeft = sLeft;
        sPrevTop = sTop;
        break;
      case MotionEvent.ACTION_UP:
      default:
        screenMatrix.postTranslate(sLeft, sTop);
        break;
    }
    return sTouchProcessor.onTouchEvent(event, screenMatrix);
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
