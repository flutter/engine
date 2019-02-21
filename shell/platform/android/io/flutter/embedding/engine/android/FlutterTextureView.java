// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.android;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;

import io.flutter.embedding.engine.renderer.FlutterRenderer;

public class FlutterTextureView extends TextureView {
  private static final String TAG = "FlutterTextureView";

  private boolean isSurfaceAvailableForRendering = false;
  private boolean isAttachedToFlutterRenderer = false;
  @Nullable
  private FlutterRenderer flutterRenderer;

  // Connects the {@code SurfaceTexture} beneath this {@code TextureView} with Flutter's native code.
  // Callbacks are received by this Object and then those messages are forwarded to our
  // FlutterRenderer, and then on to the JNI bridge over to native Flutter code.
  private final SurfaceTextureListener surfaceTextureListener = new SurfaceTextureListener() {
    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
      Log.d(TAG, "SurfaceTextureListener.onSurfaceTextureAvailable()");
      isSurfaceAvailableForRendering = true;

      // If we're already attached to a FlutterRenderer then we're now attached to both a renderer
      // and the Android window, so we can begin rendering now.
      if (isAttachedToFlutterRenderer) {
        Log.d(TAG, "Already attached to renderer. Notifying of surface creation.");
        connectSurfaceToRenderer();
      }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
      if (isAttachedToFlutterRenderer) {
        changeSurfaceSize(width, height);
      }
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
      // Invoked every time a new frame is available. We don't care.
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
      Log.d(TAG, "SurfaceTextureListener.onSurfaceTextureDestroyed()");
      isSurfaceAvailableForRendering = false;

      // If we're attached to a FlutterRenderer then we need to notify it that our SurfaceTexture
      // has been destroyed.
      if (isAttachedToFlutterRenderer) {
        disconnectSurfaceFromRenderer();
      }

      // Return true to indicate that no further painting will take place
      // within this SurfaceTexture.
      return true;
    }
  };

  /**
   * Constructs a {@code FlutterTextureView} programmatically, without any XML attributes.
   */
  public FlutterTextureView(Context context) {
    this(context, null);
  }

  /**
   * Constructs a {@code FlutterTextureView} in an XML-inflation-compliant manner.
   */
  public FlutterTextureView(Context context, AttributeSet attrs) {
    super(context, attrs);
    init();
  }

  private void init() {
    // Listen for when our underlying SurfaceTexture becomes available, changes size, or
    // gets destroyed, and take the appropriate actions.
    setSurfaceTextureListener(surfaceTextureListener);
  }

  /**
   * Invoked when this {@code FlutterTextureView} is connected to a {@link FlutterRenderer} as
   * a {@link FlutterRenderer.RenderSurface}.
   *
   * If an Android {@link SurfaceTexture} is available, this method will begin rendering
   * {@link FlutterRenderer}'s Flutter UI to this {@code FlutterTextureView}.
   *
   * If no Android {@link SurfaceTexture} is available yet, this {@code FlutterSurfaceView}
   * will wait until a {@link SurfaceTexture} becomes available and then begin rendering.
   */
  public void onAttachedToRenderer(@NonNull FlutterRenderer flutterRenderer) {
    if (this.flutterRenderer != null) {
      this.flutterRenderer.detachFromRenderSurface();
    }

    this.flutterRenderer = flutterRenderer;
    isAttachedToFlutterRenderer = true;

    // If we're already attached to an Android window then we're now attached to both a renderer
    // and the Android window. We can begin rendering now.
    if (isSurfaceAvailableForRendering) {
      connectSurfaceToRenderer();
    }
  }

  /**
   * Invoked when this {@code FlutterTextureView} is no longer connected to a {@link FlutterRenderer}
   * as a {@link FlutterRenderer.RenderSurface}.
   *
   * This method will cease any on-going rendering from Flutter to this {@code FlutterTextureView}.
   */
  public void onDetachedFromRenderer() {
    if (flutterRenderer != null) {
      // If we're attached to an Android window then we were rendering a Flutter UI. Now that
      // this FlutterTextureView is detached from the FlutterRenderer, we need to stop rendering.
      if (isAttachedToWindow()) {
        disconnectSurfaceFromRenderer();
      }

      flutterRenderer = null;
      isAttachedToFlutterRenderer = false;
    } else {
      Log.w(TAG, "detachFromRenderer() invoked when no FlutterRenderer was attached.");
    }
  }

  // FlutterRenderer and getSurfaceTexture() must both be non-null.
  private void connectSurfaceToRenderer() {
    if (flutterRenderer == null || getSurfaceTexture() == null) {
      throw new IllegalStateException("connectSurfaceToRenderer() should only be called when flutterRenderer and getSurfaceTexture() are non-null.");
    }

    flutterRenderer.surfaceCreated(new Surface(getSurfaceTexture()));
  }

  // FlutterRenderer must be non-null.
  private void changeSurfaceSize(int width, int height) {
    if (flutterRenderer == null) {
      throw new IllegalStateException("changeSurfaceSize() should only be called when flutterRenderer is non-null.");
    }

    flutterRenderer.surfaceChanged(width, height);
  }

  // FlutterRenderer must be non-null.
  private void disconnectSurfaceFromRenderer() {
    if (flutterRenderer == null) {
      throw new IllegalStateException("disconnectSurfaceFromRenderer() should only be called when flutterRenderer is non-null.");
    }

    flutterRenderer.surfaceDestroyed();
  }
}
