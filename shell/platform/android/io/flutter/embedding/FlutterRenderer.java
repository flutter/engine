package io.flutter.embedding;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.support.annotation.NonNull;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicLong;

import io.flutter.view.TextureRegistry;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class FlutterRenderer implements TextureRegistry {

  private final Set<OnFirstFrameRenderedListener> firstFrameListeners = new CopyOnWriteArraySet<>();
  private final FlutterEngine flutterEngine;
  private final FlutterJNI flutterJNI;
  private final long nativeObjectReference;
  private RenderSurface renderSurface;

  FlutterRenderer(
      @NonNull FlutterEngine flutterEngine,
      @NonNull FlutterJNI flutterJNI,
      long nativeObjectReference) {
    this.flutterEngine = flutterEngine;
    this.flutterJNI = flutterJNI;
    this.nativeObjectReference = nativeObjectReference;
  }

  public void attachToRenderSurface(@NonNull RenderSurface renderSurface) {
    // TODO(mattcarroll): what is our desired behavior when attaching to an already attached renderer?
    if (this.renderSurface != null) {
      detachFromRenderSurface();
    }

    this.renderSurface = renderSurface;
  }

  public void detachFromRenderSurface() {
    // TODO(mattcarroll): do we care if we're asked to detach without first being attached?
    if (this.renderSurface != null) {
      this.renderSurface = null;
    }
  }

  public void addOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.add(listener);
  }

  public void removeOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.remove(listener);
  }

  private void notifyFirstFrameListeners() {
    for (OnFirstFrameRenderedListener listener : firstFrameListeners) {
      listener.onFirstFrameRendered();
    }
  }

  //------ START TextureRegistry IMPLEMENTATION -----
  private final AtomicLong nextTextureId = new AtomicLong(0L);

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
      this.surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
        @Override
        public void onFrameAvailable(SurfaceTexture texture) {
          markTextureFrameAvailable(SurfaceTextureRegistryEntry.this.id);
        }
      });
    }

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
      released = true;
      unregisterTexture(id);
      surfaceTexture.release();
    }
  }
  //------ END TextureRegistry IMPLEMENTATION ----

  //------ START MIGRATION FROM FlutterEngine to FlutterRenderer -----
  public void surfaceCreated(Surface surface, int backgroundColor) {
    flutterJNI.nativeSurfaceCreated(nativeObjectReference, surface, backgroundColor);
  }

  public void surfaceChanged(int width, int height) {
    flutterJNI.nativeSurfaceChanged(nativeObjectReference, width, height);
  }

  public void surfaceDestroyed() {
    flutterJNI.nativeSurfaceDestroyed(nativeObjectReference);
  }

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

  public Bitmap getBitmap() {
    return flutterJNI.nativeGetBitmap(nativeObjectReference);
  }

  public void dispatchPointerDataPacket(ByteBuffer buffer, int position) {
    flutterJNI.nativeDispatchPointerDataPacket(nativeObjectReference, buffer, position);
  }

  public void registerTexture(long textureId, SurfaceTexture surfaceTexture) {
    flutterJNI.nativeRegisterTexture(nativeObjectReference, textureId, surfaceTexture);
  }

  public void markTextureFrameAvailable(long textureId) {
    flutterJNI.nativeMarkTextureFrameAvailable(nativeObjectReference, textureId);
  }

  public void unregisterTexture(long textureId) {
    flutterJNI.nativeUnregisterTexture(nativeObjectReference, textureId);
  }

  public boolean isSoftwareRenderingEnabled() {
    return flutterJNI.nativeGetIsSoftwareRenderingEnabled();
  }

  public void setAccessibilityFeatures(int flags) {
    flutterJNI.nativeSetAccessibilityFeatures(nativeObjectReference, flags);
  }

  public void setSemanticsEnabled(boolean enabled) {
    flutterJNI.nativeSetSemanticsEnabled(nativeObjectReference, enabled);
  }

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
  //------ END MIGRATION FROM FlutterEngine to FlutterRenderer ----

  //------ START PACKAGE PRIVATE MESSAGES FROM FlutterEngine ------
  void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
    if (renderSurface != null) {
      renderSurface.updateCustomAccessibilityActions(buffer, strings);
    }
  }

  void updateSemantics(ByteBuffer buffer, String[] strings) {
    if (renderSurface != null) {
      renderSurface.updateSemantics(buffer, strings);
    }
  }

  void onFirstFrameRendered() {
    if (renderSurface != null) {
      renderSurface.onFirstFrameRendered();
      notifyFirstFrameListeners();
    }
  }
  //------ END PACKAGE PRIVATE MESSAGES FROM FlutterEngine ------

  public interface OnFirstFrameRenderedListener {
    void onFirstFrameRendered();
  }

  public interface RenderSurface {
    void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings);

    void updateSemantics(ByteBuffer buffer, String[] strings);

    void onFirstFrameRendered();
  }
}
