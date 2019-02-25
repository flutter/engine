// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.android;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.FrameLayout;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.renderer.FlutterRenderer;

/**
 * Displays a Flutter UI on an Android device.
 * <p>
 * A {@code FlutterView}'s UI is painted by a corresponding {@link FlutterEngine}.
 * <p>
 * A {@code FlutterView} can operate in 2 different {@link RenderMode}s:
 * <ol>
 *   <li>{@link RenderMode#surface}, which paints a Flutter UI to a {@link android.view.SurfaceView}.
 *   This mode has the best performance, but a {@code FlutterView} in this mode cannot be positioned
 *   between 2 other Android {@code View}s in the z-index, nor can it be animated/transformed.
 *   Unless the special capabilities of a {@link android.graphics.SurfaceTexture} are required,
 *   developers should strongly prefer this render mode.</li>
 *   <li>{@link RenderMode#texture}, which paints a Flutter UI to a {@link android.graphics.SurfaceTexture}.
 *   This mode is not as performant as {@link RenderMode#surface}, but a {@code FlutterView} in this
 *   mode can be animated and transformed, as well as positioned in the z-index between 2+ other
 *   Android {@code Views}. Unless the special capabilities of a {@link android.graphics.SurfaceTexture}
 *   are required, developers should strongly prefer the {@link RenderMode#surface} render mode.</li>
 * </ol>
 * See <a>https://source.android.com/devices/graphics/arch-tv#surface_or_texture</a> for more
 * information comparing {@link android.view.SurfaceView} and {@link android.view.TextureView}.
 */
public class FlutterView extends FrameLayout {
  private static final String TAG = "FlutterView";

  // View configuration
  @NonNull
  private RenderMode renderMode;

  // View structure
  @Nullable
  private FlutterRenderer.RenderSurface renderSurface;

  // Flutter engine
  @Nullable
  private FlutterEngine flutterEngine;
  private boolean isAttachedToFlutterEngine = false;

  /**
   * Constructs a {@code FlutterSurfaceView} programmatically, without any XML attributes.
   *
   * {@link #renderMode} defaults to {@link RenderMode#surface}.
   */
  public FlutterView(@NonNull Context context) {
    this(context, null, null);
  }

  /**
   * Constructs a {@code FlutterSurfaceView} programmatically, without any XML attributes,
   * and allows selection of a {@link #renderMode}.
   */
  public FlutterView(@NonNull Context context, @NonNull RenderMode renderMode) {
    this(context, null, renderMode);
  }

  /**
   * Constructs a {@code FlutterSurfaceView} in an XML-inflation-compliant manner.
   *
   * // TODO(mattcarroll): expose renderMode in XML when build system supports R.attr
   */
  public FlutterView(@NonNull Context context, @Nullable AttributeSet attrs) {
    this(context, attrs, null);
  }

  private FlutterView(@NonNull Context context, @Nullable AttributeSet attrs, @Nullable RenderMode renderMode) {
    super(context, attrs);

    this.renderMode = renderMode == null ? RenderMode.surface : renderMode;

    init();
  }

  private void init() {
    Log.d(TAG, "Initializing FlutterView");

    switch (renderMode) {
      case surface:
        Log.d(TAG, "Internally creating a FlutterSurfaceView.");
        FlutterSurfaceView flutterSurfaceView = new FlutterSurfaceView(getContext());
        renderSurface = flutterSurfaceView;
        addView(flutterSurfaceView);
        break;
      case texture:
        Log.d(TAG, "Internally creating a FlutterTextureView.");
        FlutterTextureView flutterTextureView = new FlutterTextureView(getContext());
        renderSurface = flutterTextureView;
        addView(flutterTextureView);
        break;
    }
  }

  /**
   * Connects this {@code FlutterView} to the given {@link FlutterEngine}.
   *
   * Once invoked, the Flutter UI painted by the given {@link FlutterEngine} will be
   * displayed by this {@code FlutterView}. Additionally, user touch events, accessibility
   * events, keyboard events, and more will be forwarded from this {@code FlutterView}
   * to the attached {@link FlutterEngine}.
   *
   * See {@link #detachFromFlutterEngine()} for information on how to detach from a
   * {@link FlutterEngine}.
   */
  public void attachToFlutterEngine(@NonNull FlutterEngine flutterEngine) {
    if (isAttachedToFlutterEngine) {
      if (flutterEngine == this.flutterEngine) {
        // We are already attached to this FlutterEngine
        return;
      }

      // Detach from a previous FlutterEngine so we can attach to this new one.
      detachFromFlutterEngine();
    }

    this.flutterEngine = flutterEngine;
    isAttachedToFlutterEngine = true;

    // Instruct our FlutterRenderer that we are now its designated RenderSurface.
    this.flutterEngine.getRenderer().attachToRenderSurface(renderSurface);
  }

  /**
   * Disconnects this {@code FlutterView} from a previously attached {@link FlutterEngine}.
   *
   * Once invoked, the UI of this {@code FlutterView} will be cleared, and all touch events,
   * accessibility events, keyboard events, etc. will no longer be forwarded to the previously
   * attached {@link FlutterEngine}.
   *
   * See {@link #attachToFlutterEngine(FlutterEngine)} for information on how to attach a
   * {@link FlutterEngine}.
   */
  public void detachFromFlutterEngine() {
    if (!isAttachedToFlutterEngine) {
      return;
    }
    Log.d(TAG, "Detaching from Flutter Engine");

    // Instruct our FlutterRenderer that we are no longer interested in being its RenderSurface.
    flutterEngine.getRenderer().detachFromRenderSurface();
    flutterEngine = null;

    isAttachedToFlutterEngine = false;

    // TODO(mattcarroll): clear the surface when JNI doesn't blow up
//    if (isSurfaceAvailableForRendering) {
//      Canvas canvas = surfaceHolder.lockCanvas();
//      canvas.drawColor(Color.RED);
//      surfaceHolder.unlockCanvasAndPost(canvas);
//    }
  }

  /**
   * Render modes for a {@link FlutterView}.
   */
  public enum RenderMode {
    /**
     * {@code RenderMode}, which paints a Flutter UI to a {@link android.view.SurfaceView}.
     * This mode has the best performance, but a {@code FlutterView} in this mode cannot be positioned
     * between 2 other Android {@code View}s in the z-index, nor can it be animated/transformed.
     * Unless the special capabilities of a {@link android.graphics.SurfaceTexture} are required,
     * developers should strongly prefer this render mode.
     */
    surface,
    /**
     * {@code RenderMode}, which paints a Flutter UI to a {@link android.graphics.SurfaceTexture}.
     * This mode is not as performant as {@link RenderMode#surface}, but a {@code FlutterView} in this
     * mode can be animated and transformed, as well as positioned in the z-index between 2+ other
     * Android {@code Views}. Unless the special capabilities of a {@link android.graphics.SurfaceTexture}
     * are required, developers should strongly prefer the {@link RenderMode#surface} render mode.
     */
    texture
  }
}
