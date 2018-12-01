// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

import io.flutter.embedding.engine.dart.PlatformMessageHandler;
import io.flutter.embedding.engine.FlutterEngine.EngineHandler;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.renderer.OnFirstFrameRenderedListener;
import io.flutter.plugin.common.BinaryMessenger;

public class FlutterJNI {
  private static final String TAG = "FlutterJNI";

  private FlutterRenderer.RenderSurface renderSurface;
  private PlatformMessageHandler platformMessageHandler;
  private @Nullable EngineHandler engineHandler;
  private final Set<OnFirstFrameRenderedListener> firstFrameListeners = new CopyOnWriteArraySet<>();

  public void setRenderSurface(@Nullable FlutterRenderer.RenderSurface renderSurface) {
    this.renderSurface = renderSurface;
  }

  public void setPlatformMessageHandler(@Nullable PlatformMessageHandler platformMessageHandler) {
    this.platformMessageHandler = platformMessageHandler;
  }

  public void setEngineHandler(@Nullable EngineHandler engineHandler) {
    this.engineHandler = engineHandler;
  }

  public void addOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.add(listener);
  }

  public void removeOnFirstFrameRenderedListener(@NonNull OnFirstFrameRenderedListener listener) {
    firstFrameListeners.remove(listener);
  }

  //------ START RENDER SURFACE CALLBACKS -----
  // Called by native to update the semantics/accessibility tree.
  @SuppressWarnings("unused")
  public void updateSemantics(ByteBuffer buffer, String[] strings) {
    Log.d(TAG, "updateSemantics()");
    if (renderSurface != null) {
      renderSurface.updateSemantics(buffer, strings);
    }
  }

  // Called by native to update the custom accessibility actions.
  @SuppressWarnings("unused")
  public void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
    Log.d(TAG, "updateCustomAccessibilityActions()");
    if (renderSurface != null) {
      renderSurface.updateCustomAccessibilityActions(buffer, strings);
    }
  }

  // Called by native to notify first Flutter frame rendered.
  @SuppressWarnings("unused")
  private void onFirstFrame() {
    Log.d(TAG, "onFirstFrame()");
    if (renderSurface != null) {
      renderSurface.onFirstFrameRendered();
    }

    for (OnFirstFrameRenderedListener listener : firstFrameListeners) {
      listener.onFirstFrameRendered();
    }
  }
  //------ END RENDER SURFACE CALLBACKS ------

  //------ START PLATFORM MESSAGE CALLBACKS ----
  @SuppressWarnings("unused")
  private void handlePlatformMessage(final String channel, byte[] message, final int replyId) {
    if (platformMessageHandler != null) {
      platformMessageHandler.handlePlatformMessage(channel, message, replyId);
    }
  }

  // Called by native to respond to a platform message that we sent.
  @SuppressWarnings("unused")
  private void handlePlatformMessageResponse(int replyId, byte[] reply) {
    if (platformMessageHandler != null) {
      platformMessageHandler.handlePlatformMessageResponse(replyId, reply);
    }
  }
  //------ END PLATFORM MESSAGE CALLBACKS ----

  //----- Start from FlutterView -----
  public native void nativeSurfaceCreated(long nativePlatformViewAndroid, Surface surface);

  public native void nativeSurfaceChanged(long nativePlatformViewAndroid,
                                          int width,
                                          int height);

  public native void nativeSurfaceDestroyed(long nativePlatformViewAndroid);

  public native void nativeSetViewportMetrics(long nativePlatformViewAndroid,
                                              float devicePixelRatio,
                                              int physicalWidth,
                                              int physicalHeight,
                                              int physicalPaddingTop,
                                              int physicalPaddingRight,
                                              int physicalPaddingBottom,
                                              int physicalPaddingLeft,
                                              int physicalViewInsetTop,
                                              int physicalViewInsetRight,
                                              int physicalViewInsetBottom,
                                              int physicalViewInsetLeft);

  public native Bitmap nativeGetBitmap(long nativePlatformViewAndroid);

  public native void nativeDispatchPointerDataPacket(long nativePlatformViewAndroid,
                                                     ByteBuffer buffer,
                                                     int position);

  public native void nativeDispatchSemanticsAction(long nativePlatformViewAndroid,
                                                   int id,
                                                   int action,
                                                   ByteBuffer args,
                                                   int argsPosition);

  public native void nativeSetSemanticsEnabled(long nativePlatformViewAndroid, boolean enabled);

  public native void nativeSetAccessibilityFeatures(long nativePlatformViewAndroid, int flags);

  public native boolean nativeGetIsSoftwareRenderingEnabled();

  public native void nativeRegisterTexture(long nativePlatformViewAndroid, long textureId, SurfaceTexture surfaceTexture);

  public native void nativeMarkTextureFrameAvailable(long nativePlatformViewAndroid, long textureId);

  public native void nativeUnregisterTexture(long nativePlatformViewAndroid, long textureId);
  //------- End from FlutterView -----

  //------ Start from FlutterNativeView ----
  public native long nativeAttach(FlutterJNI flutterJNI, boolean isBackgroundView);
  public native void nativeDestroy(long nativePlatformViewAndroid);
  public native void nativeDetach(long nativePlatformViewAndroid);

  public native void nativeRunBundleAndSnapshotFromLibrary(
      long nativePlatformViewAndroid,
      String bundlePath,
      String defaultPath,
      String entrypoint,
      String libraryUrl,
      AssetManager manager
  );

  public native String nativeGetObservatoryUri();

  // Send an empty platform message to Dart.
  public native void nativeDispatchEmptyPlatformMessage(
      long nativePlatformViewAndroid,
      String channel,
      int responseId
  );

  // Send a data-carrying platform message to Dart.
  public native void nativeDispatchPlatformMessage(
      long nativePlatformViewAndroid,
      String channel,
      ByteBuffer message,
      int position,
      int responseId
  );

  // Send an empty response to a platform message received from Dart.
  public native void nativeInvokePlatformMessageEmptyResponseCallback(
      long nativePlatformViewAndroid,
      int responseId
  );

  // Send a data-carrying response to a platform message received from Dart.
  public native void nativeInvokePlatformMessageResponseCallback(
      long nativePlatformViewAndroid,
      int responseId,
      ByteBuffer message,
      int position
  );
  //------ End from FlutterNativeView ----

  //------ Start from Engine ---
  @SuppressWarnings("unused")
  private void onPreEngineRestart() {
    if (engineHandler == null) {
      return;
    }
    engineHandler.onPreEngineRestart();
  }
  //------ End from Engine ---
}
