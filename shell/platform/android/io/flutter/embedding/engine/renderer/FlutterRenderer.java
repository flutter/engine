// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.renderer;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicLong;

import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.view.TextureRegistry;

/**
 * {@code FlutterRenderer} works in tandem with a provided {@link RenderSurface} to create an
 * interactive Flutter UI.
 *
 * {@code FlutterRenderer} manages textures for rendering, and forwards messages to native Flutter
 * code via JNI. The corresponding {@link RenderSurface} is used as a delegate to carry out
 * certain actions on behalf of this {@code FlutterRenderer} within an Android view hierarchy.
 *
 * {@link FlutterView} is an implementation of a {@link RenderSurface}.
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class FlutterRenderer implements TextureRegistry {

  private final FlutterJNI flutterJNI;
  private final long nativeObjectReference;
  private final AtomicLong nextTextureId = new AtomicLong(0L);
  private RenderSurface renderSurface;

  public FlutterRenderer(@NonNull FlutterJNI flutterJNI, long nativeObjectReference) {
    this.flutterJNI = flutterJNI;
    this.nativeObjectReference = nativeObjectReference;
  }

  public void attachToRenderSurface(@NonNull RenderSurface renderSurface) {
    // TODO(mattcarroll): what is our desired behavior when attaching to an already attached renderer?
    if (this.renderSurface != null) {
      detachFromRenderSurface();
    }

    this.renderSurface = renderSurface;
    this.flutterJNI.setRenderSurface(renderSurface);
  }

  public void detachFromRenderSurface() {
    // TODO(mattcarroll): do we care if we're asked to detach without first being attached?
    if (this.renderSurface != null) {
      surfaceDestroyed();
      this.renderSurface = null;
      this.flutterJNI.setRenderSurface(null);
    }
  }

  public void addOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    flutterJNI.addOnFirstFrameRenderedListener(listener);
  }

  public void removeOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    flutterJNI.removeOnFirstFrameRenderedListener(listener);
  }

  //------ START TextureRegistry IMPLEMENTATION -----
  // TODO(mattcarroll): this method probably shouldn't be public. It's part of an obscure relationship
  //   with PlatformViewsController. Re-evaluate that relationship and see if we can get rid of
  //   PlatformViewsController.
  //   However, I did find that this method is called from the Camera plugin.
  // TODO(mattcarroll): detachFromGLContext requires API 16. what should we do for earlier APIs?
  @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
  @Override
  public SurfaceTextureEntry createSurfaceTexture() {
    final SurfaceTexture surfaceTexture = new SurfaceTexture(0);
    surfaceTexture.detachFromGLContext();
    final SurfaceTextureRegistryEntry entry = new SurfaceTextureRegistryEntry(
        nextTextureId.getAndIncrement(),
        surfaceTexture
    );
    registerTexture(entry.id(), surfaceTexture);
    return entry;
  }

  final class SurfaceTextureRegistryEntry implements TextureRegistry.SurfaceTextureEntry {
    private final long id;
    private final SurfaceTexture surfaceTexture;
    private boolean released;

    SurfaceTextureRegistryEntry(long id, SurfaceTexture surfaceTexture) {
      this.id = id;
      this.surfaceTexture = surfaceTexture;

      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        // The callback relies on being executed on the UI thread (unsynchronised read of mNativeView
        // and also the engine code check for platform thread in Shell::OnPlatformViewMarkTextureFrameAvailable),
        // so we explicitly pass a Handler for the current thread.
        this.surfaceTexture.setOnFrameAvailableListener(onFrameListener, new Handler());
      } else {
        // Android documentation states that the listener can be called on an arbitrary thread.
        // But in practice, versions of Android that predate the newer API will call the listener
        // on the thread where the SurfaceTexture was constructed.
        this.surfaceTexture.setOnFrameAvailableListener(onFrameListener);
      }
    }

    private SurfaceTexture.OnFrameAvailableListener onFrameListener = new SurfaceTexture.OnFrameAvailableListener() {
      @Override
      public void onFrameAvailable(SurfaceTexture texture) {
        if (released) {
          // Even though we make sure to unregister the callback before releasing, as of Android O
          // SurfaceTexture has a data race when accessing the callback, so the callback may
          // still be called by a stale reference after released==true and mNativeView==null.
          return;
        }
        markTextureFrameAvailable(id);
      }
    };

    @Override
    public SurfaceTexture surfaceTexture() {
      return surfaceTexture;
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
      unregisterTexture(id);
      surfaceTexture.release();
      released = true;
    }
  }
  //------ END TextureRegistry IMPLEMENTATION ----

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void surfaceCreated(Surface surface) {
    flutterJNI.nativeSurfaceCreated(nativeObjectReference, surface);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void surfaceChanged(int width, int height) {
    flutterJNI.nativeSurfaceChanged(nativeObjectReference, width, height);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void surfaceDestroyed() {
    flutterJNI.nativeSurfaceDestroyed(nativeObjectReference);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void setViewportMetrics(float devicePixelRatio,
                                 int physicalWidth,
                                 int physicalHeight,
                                 int physicalPaddingTop,
                                 int physicalPaddingRight,
                                 int physicalPaddingBottom,
                                 int physicalPaddingLeft,
                                 int physicalViewInsetTop,
                                 int physicalViewInsetRight,
                                 int physicalViewInsetBottom,
                                 int physicalViewInsetLeft) {
    flutterJNI.nativeSetViewportMetrics(
        nativeObjectReference,
        devicePixelRatio,
        physicalWidth,
        physicalHeight,
        physicalPaddingTop,
        physicalPaddingRight,
        physicalPaddingBottom,
        physicalPaddingLeft,
        physicalViewInsetTop,
        physicalViewInsetRight,
        physicalViewInsetBottom,
        physicalViewInsetLeft
    );
  }

  // TODO(mattcarroll): why does this method exist?
  public Bitmap getBitmap() {
    return flutterJNI.nativeGetBitmap(nativeObjectReference);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void dispatchPointerDataPacket(ByteBuffer buffer, int position) {
    flutterJNI.nativeDispatchPointerDataPacket(nativeObjectReference, buffer, position);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  private void registerTexture(long textureId, SurfaceTexture surfaceTexture) {
    flutterJNI.nativeRegisterTexture(nativeObjectReference, textureId, surfaceTexture);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  private void markTextureFrameAvailable(long textureId) {
    flutterJNI.nativeMarkTextureFrameAvailable(nativeObjectReference, textureId);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  private void unregisterTexture(long textureId) {
    flutterJNI.nativeUnregisterTexture(nativeObjectReference, textureId);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public boolean isSoftwareRenderingEnabled() {
    return flutterJNI.nativeGetIsSoftwareRenderingEnabled();
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void setAccessibilityFeatures(int flags) {
    flutterJNI.nativeSetAccessibilityFeatures(nativeObjectReference, flags);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void setSemanticsEnabled(boolean enabled) {
    flutterJNI.nativeSetSemanticsEnabled(nativeObjectReference, enabled);
  }

  // TODO(mattcarroll): what exactly is this method intended to do?
  public void dispatchSemanticsAction(int id,
                                      int action,
                                      ByteBuffer args,
                                      int argsPosition) {
    flutterJNI.nativeDispatchSemanticsAction(
        nativeObjectReference,
        id,
        action,
        args,
        argsPosition
    );
  }

  /**
   * Delegate used in conjunction with a {@link FlutterRenderer} to create an interactive Flutter
   * UI.
   *
   * A {@code RenderSurface} is responsible for carrying out behaviors that are needed by a
   * corresponding {@link FlutterRenderer}, e.g., {@link #updateSemantics(ByteBuffer, String[])}.
   *
   * A {@code RenderSurface} also receives callbacks for important events, e.g.,
   * {@link #onFirstFrameRendered()}.
   */
  public interface RenderSurface {
    // TODO(mattcarroll): what is this method supposed to do?
    void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings);

    // TODO(mattcarroll): what is this method supposed to do?
    void updateSemantics(ByteBuffer buffer, String[] strings);

    /**
     * The {@link FlutterRenderer} corresponding to this {@code RenderSurface} has painted its
     * first frame since being initialized.
     *
     * "Initialized" refers to Flutter engine initialization, not the first frame after attaching
     * to the {@link FlutterRenderer}. Therefore, the first frame may have already rendered by
     * the time a {@code RenderSurface} has called {@link #attachToRenderSurface(RenderSurface)}
     * on a {@link FlutterRenderer}. In such a situation, {@link #onFirstFrameRendered()} will
     * never be called.
     */
    void onFirstFrameRendered();
  }
}
