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
  private final Set<OnFirstFrameRenderedListener> firstFrameListeners = new CopyOnWriteArraySet<>();
  private RenderSurface renderSurface;

  FlutterRenderer(@NonNull FlutterJNI flutterJNI, long nativeObjectReference) {
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
      surfaceDestroyed();
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

  // TODO(mattcarroll): change the JNI code to call these directly rather than forward these calls from FlutterEngine
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
    }
    notifyFirstFrameListeners();
  }
  //------ END PACKAGE PRIVATE MESSAGES FROM FlutterEngine ------

  public interface OnFirstFrameRenderedListener {
    /**
     * A {@link FlutterRenderer} has painted its first frame since being initialized.
     *
     * This method will not be invoked if this listener is added after the first frame is rendered.
     */
    void onFirstFrameRendered();
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
