// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.renderer;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.view.Surface;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.view.TextureRegistry;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Represents the rendering responsibilities of a {@code FlutterEngine}.
 *
 * <p>{@code FlutterRenderer} works in tandem with a provided {@link RenderSurface} to paint Flutter
 * pixels to an Android {@code View} hierarchy.
 *
 * <p>{@code FlutterRenderer} manages textures for rendering, and forwards some Java calls to native
 * Flutter code via JNI. The corresponding {@link RenderSurface} provides the Android {@link
 * Surface} that this renderer paints.
 *
 * <p>{@link io.flutter.embedding.android.FlutterSurfaceView} and {@link
 * io.flutter.embedding.android.FlutterTextureView} are implementations of {@link RenderSurface}.
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class FlutterRenderer implements TextureRegistry {
  private static final String TAG = "FlutterRenderer";

  @NonNull private final FlutterJNI flutterJNI;
  @NonNull private final AtomicLong nextTextureId = new AtomicLong(0L);
  @Nullable private Surface surface;
  private boolean isDisplayingFlutterUi = false;

  @NonNull
  private final FlutterUiDisplayListener flutterUiDisplayListener =
      new FlutterUiDisplayListener() {
        @Override
        public void onFlutterUiDisplayed() {
          isDisplayingFlutterUi = true;
        }

        @Override
        public void onFlutterUiNoLongerDisplayed() {
          isDisplayingFlutterUi = false;
        }
      };

  public FlutterRenderer(@NonNull FlutterJNI flutterJNI) {
    this.flutterJNI = flutterJNI;
    this.flutterJNI.addIsDisplayingFlutterUiListener(flutterUiDisplayListener);
  }

  /**
   * Returns true if this {@code FlutterRenderer} is painting pixels to an Android {@code View}
   * hierarchy, false otherwise.
   */
  public boolean isDisplayingFlutterUi() {
    return isDisplayingFlutterUi;
  }

  /**
   * Adds a listener that is invoked whenever this {@code FlutterRenderer} starts and stops painting
   * pixels to an Android {@code View} hierarchy.
   */
  public void addIsDisplayingFlutterUiListener(@NonNull FlutterUiDisplayListener listener) {
    flutterJNI.addIsDisplayingFlutterUiListener(listener);

    if (isDisplayingFlutterUi) {
      listener.onFlutterUiDisplayed();
    }
  }

  /**
   * Removes a listener added via {@link
   * #addIsDisplayingFlutterUiListener(FlutterUiDisplayListener)}.
   */
  public void removeIsDisplayingFlutterUiListener(@NonNull FlutterUiDisplayListener listener) {
    flutterJNI.removeIsDisplayingFlutterUiListener(listener);
  }

  // ------ START TextureRegistry IMPLEMENTATION -----
  /**
   * Creates and returns a new {@link SurfaceTexture} that is also made available to Flutter code.
   */
  @Override
  public SurfaceTextureEntry createSurfaceTexture() {
    Log.v(TAG, "Creating a SurfaceTexture.");
    final SurfaceTexture surfaceTexture = new SurfaceTexture(0);
    surfaceTexture.detachFromGLContext();
    final SurfaceTextureRegistryEntry entry =
        new SurfaceTextureRegistryEntry(nextTextureId.getAndIncrement(), surfaceTexture);
    Log.v(TAG, "New SurfaceTexture ID: " + entry.id());
    registerTexture(entry.id(), entry.textureWrapper());
    return entry;
  }

  final class SurfaceTextureRegistryEntry implements TextureRegistry.SurfaceTextureEntry {
    private final long id;
    @NonNull private final SurfaceTextureWrapper textureWrapper;
    private boolean released;

    SurfaceTextureRegistryEntry(long id, @NonNull SurfaceTexture surfaceTexture) {
      this.id = id;
      this.textureWrapper = new SurfaceTextureWrapper(surfaceTexture);

      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        // The callback relies on being executed on the UI thread (unsynchronised read of
        // mNativeView
        // and also the engine code check for platform thread in
        // Shell::OnPlatformViewMarkTextureFrameAvailable),
        // so we explicitly pass a Handler for the current thread.
        this.surfaceTexture().setOnFrameAvailableListener(onFrameListener, new Handler());
      } else {
        // Android documentation states that the listener can be called on an arbitrary thread.
        // But in practice, versions of Android that predate the newer API will call the listener
        // on the thread where the SurfaceTexture was constructed.
        this.surfaceTexture().setOnFrameAvailableListener(onFrameListener);
      }
    }

    private SurfaceTexture.OnFrameAvailableListener onFrameListener =
        new SurfaceTexture.OnFrameAvailableListener() {
          @Override
          public void onFrameAvailable(@NonNull SurfaceTexture texture) {
            if (released || !flutterJNI.isAttached()) {
              // Even though we make sure to unregister the callback before releasing, as of
              // Android O, SurfaceTexture has a data race when accessing the callback, so the
              // callback may still be called by a stale reference after released==true and
              // mNativeView==null.
              return;
            }
            markTextureFrameAvailable(id);
          }
        };

    @NonNull
    public SurfaceTextureWrapper textureWrapper() {
      return textureWrapper;
    }

    @Override
    @NonNull
    public SurfaceTexture surfaceTexture() {
      return textureWrapper.surfaceTexture();
    }

    @Override
    public long id() {
      return id;
    }

    @Override
    public void release() {
      if (released) {
        return;
      }
      Log.v(TAG, "Releasing a SurfaceTexture (" + id + ").");
      textureWrapper.release();
      unregisterTexture(id);
      released = true;
    }
  }
  // ------ END TextureRegistry IMPLEMENTATION ----

  /**
   * Notifies Flutter that the given {@code surface} was created and is available for Flutter
   * rendering.
   *
   * <p>See {@link android.view.SurfaceHolder.Callback} and {@link
   * android.view.TextureView.SurfaceTextureListener}
   */
  public void startRenderingToSurface(@NonNull Surface surface) {
    if (this.surface != null) {
      stopRenderingToSurface();
    }

    this.surface = surface;

    flutterJNI.onSurfaceCreated(surface);
  }

  /**
   * Swaps the {@link Surface} used to render the current frame.
   *
   * <p>In hybrid composition, the root surfaces changes from {@link
   * android.view.SurfaceHolder#getSurface()} to {@link android.media.ImageReader#getSurface()} when
   * a platform view is in the current frame.
   */
  public void swapSurface(@NonNull Surface surface) {
    this.surface = surface;
    flutterJNI.onSurfaceWindowChanged(surface);
  }

  /**
   * Notifies Flutter that a {@code surface} previously registered with {@link
   * #startRenderingToSurface(Surface)} has changed size to the given {@code width} and {@code
   * height}.
   *
   * <p>See {@link android.view.SurfaceHolder.Callback} and {@link
   * android.view.TextureView.SurfaceTextureListener}
   */
  public void surfaceChanged(int width, int height) {
    flutterJNI.onSurfaceChanged(width, height);
  }

  /**
   * Notifies Flutter that a {@code surface} previously registered with {@link
   * #startRenderingToSurface(Surface)} has been destroyed and needs to be released and cleaned up
   * on the Flutter side.
   *
   * <p>See {@link android.view.SurfaceHolder.Callback} and {@link
   * android.view.TextureView.SurfaceTextureListener}
   */
  public void stopRenderingToSurface() {
    flutterJNI.onSurfaceDestroyed();

    surface = null;

    // TODO(mattcarroll): the source of truth for this call should be FlutterJNI, which is where
    // the call to onFlutterUiDisplayed() comes from. However, no such native callback exists yet,
    // so until the engine and FlutterJNI are configured to call us back when rendering stops,
    // we will manually monitor that change here.
    if (isDisplayingFlutterUi) {
      flutterUiDisplayListener.onFlutterUiNoLongerDisplayed();
    }

    isDisplayingFlutterUi = false;
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public void setViewportMetrics(@NonNull ViewportMetrics viewportMetrics) {
    Log.v(
        TAG,
        "Setting viewport metrics\n"
            + "Size: "
            + viewportMetrics.width
            + " x "
            + viewportMetrics.height
            + "\n"
            + "Padding - L: "
            + viewportMetrics.viewPaddingLeft
            + ", T: "
            + viewportMetrics.viewPaddingTop
            + ", R: "
            + viewportMetrics.viewPaddingRight
            + ", B: "
            + viewportMetrics.viewPaddingBottom
            + "\n"
            + "Insets - L: "
            + viewportMetrics.viewInsetLeft
            + ", T: "
            + viewportMetrics.viewInsetTop
            + ", R: "
            + viewportMetrics.viewInsetRight
            + ", B: "
            + viewportMetrics.viewInsetBottom
            + "\n"
            + "System Gesture Insets - L: "
            + viewportMetrics.systemGestureInsetLeft
            + ", T: "
            + viewportMetrics.systemGestureInsetTop
            + ", R: "
            + viewportMetrics.systemGestureInsetRight
            + ", B: "
            + viewportMetrics.systemGestureInsetRight
            + "\n"
            + "Display Features Count: "
            + viewportMetrics.displayFeatures.size());

    int[] displayFeaturesBounds = new int[viewportMetrics.displayFeatures.size() * 4];
    int[] displayFeaturesType = new int[viewportMetrics.displayFeatures.size()];
    int[] displayFeaturesState = new int[viewportMetrics.displayFeatures.size()];
    for(int i=0; i<viewportMetrics.displayFeatures.size(); i++) {
      DisplayFeature displayFeature = viewportMetrics.displayFeatures.get(i);
      displayFeaturesBounds[4*i] = displayFeature.bounds.left;
      displayFeaturesBounds[4*i+1] = displayFeature.bounds.top;
      displayFeaturesBounds[4*i+2] = displayFeature.bounds.right;
      displayFeaturesBounds[4*i+3] = displayFeature.bounds.bottom;
      displayFeaturesType[i] = displayFeature.type.encodedValue;
      displayFeaturesState[i] = displayFeature.state;
    }

    flutterJNI.setViewportMetrics(
        viewportMetrics.devicePixelRatio,
        viewportMetrics.width,
        viewportMetrics.height,
        viewportMetrics.viewPaddingTop,
        viewportMetrics.viewPaddingRight,
        viewportMetrics.viewPaddingBottom,
        viewportMetrics.viewPaddingLeft,
        viewportMetrics.viewInsetTop,
        viewportMetrics.viewInsetRight,
        viewportMetrics.viewInsetBottom,
        viewportMetrics.viewInsetLeft,
        viewportMetrics.systemGestureInsetTop,
        viewportMetrics.systemGestureInsetRight,
        viewportMetrics.systemGestureInsetBottom,
        viewportMetrics.systemGestureInsetLeft,
        displayFeaturesBounds,
        displayFeaturesType,
        displayFeaturesState);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  // TODO(mattcarroll): determine if this is nullable or nonnull
  public Bitmap getBitmap() {
    return flutterJNI.getBitmap();
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public void dispatchPointerDataPacket(@NonNull ByteBuffer buffer, int position) {
    flutterJNI.dispatchPointerDataPacket(buffer, position);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  private void registerTexture(long textureId, @NonNull SurfaceTextureWrapper textureWrapper) {
    flutterJNI.registerTexture(textureId, textureWrapper);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  private void markTextureFrameAvailable(long textureId) {
    flutterJNI.markTextureFrameAvailable(textureId);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  private void unregisterTexture(long textureId) {
    flutterJNI.unregisterTexture(textureId);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public boolean isSoftwareRenderingEnabled() {
    return flutterJNI.getIsSoftwareRenderingEnabled();
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public void setAccessibilityFeatures(int flags) {
    flutterJNI.setAccessibilityFeatures(flags);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public void setSemanticsEnabled(boolean enabled) {
    flutterJNI.setSemanticsEnabled(enabled);
  }

  // TODO(mattcarroll): describe the native behavior that this invokes
  public void dispatchSemanticsAction(
      int id, int action, @Nullable ByteBuffer args, int argsPosition) {
    flutterJNI.dispatchSemanticsAction(id, action, args, argsPosition);
  }

  /**
   * Mutable data structure that holds all viewport metrics properties that Flutter cares about.
   *
   * <p>All distance measurements, e.g., width, height, padding, viewInsets, are measured in device
   * pixels, not logical pixels.
   */
  public static final class ViewportMetrics {
    public float devicePixelRatio = 1.0f;
    public int width = 0;
    public int height = 0;
    public int viewPaddingTop = 0;
    public int viewPaddingRight = 0;
    public int viewPaddingBottom = 0;
    public int viewPaddingLeft = 0;
    public int viewInsetTop = 0;
    public int viewInsetRight = 0;
    public int viewInsetBottom = 0;
    public int viewInsetLeft = 0;
    public int systemGestureInsetTop = 0;
    public int systemGestureInsetRight = 0;
    public int systemGestureInsetBottom = 0;
    public int systemGestureInsetLeft = 0;
    public List<DisplayFeature> displayFeatures = List.of();
  }

  /**
   * Area of the viewport obstructed by a feature, eg. a hinge, a fold crease or camera cutout
   *
   * <p>Similar to {@link androidx.window.DisplayFeature}, used for reporting Fold / Hinge areas.
   * The reason we define our own model is that we also want to support cutouts.
   */
  public static final class DisplayFeature {
    public final Rect bounds;
    public final DisplayFeatureType type;
    public final int state;

    public DisplayFeature(Rect bounds, DisplayFeatureType type, int state) {
      this.bounds = bounds;
      this.type = type;
      this.state = state;
    }

    public DisplayFeature(Rect bounds, DisplayFeatureType type) {
      this.bounds = bounds;
      this.type = type;
      this.state = 0; // UNKNOWN
    }
  }

  /**
   * Types of display features that can obstruct the viewport.
   *
   * <p>Some, like FOLD, can be reported without actually impeding drawing on the screen. They are
   * useful for knowing where the display is bent or has a crease. The {@link DisplayFeature} bounds
   * can be 0-width in such cases.
   */
  public enum DisplayFeatureType {
    /**
     * We do not know this type of display feature yet. This can happen if WindowManager is updated
     * with new types.
     */
    UNKNOWN(0),

    /**
     * A fold in the flexible screen without a physical gap.
     * Corresponds to {@link androidx.window.DisplayFeature.TYPE_FOLD}
     */
    FOLD(1),

    /**
     * A physical separation with a hinge that allows two display panels to fold.
     * Corresponds to {@link androidx.window.DisplayFeature.TYPE_HINGE}
     */
    HINGE(2),

    /**
     * A non-functional area of the screen, usually housing cameras or sensors.
     * Corresponds to {@link android.view.DisplayCutout}
     */
    CUTOUT(3);

    public final int encodedValue;

    DisplayFeatureType(int encodedValue) {
      this.encodedValue = encodedValue;
    }
  }
}
