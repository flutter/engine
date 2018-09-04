package io.flutter.embedding;

import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.support.annotation.NonNull;
import android.view.Surface;
import io.flutter.embedding.legacy.FlutterNativeView;
import io.flutter.embedding.legacy.FlutterPluginRegistry;
import io.flutter.embedding.legacy.PluginRegistry;
import io.flutter.plugin.common.BinaryMessenger;

import java.nio.ByteBuffer;

public class FlutterEngine implements BinaryMessenger {
  private FlutterNativeView nativeView;
  private FlutterPluginRegistry pluginRegistry;

  FlutterEngine(@NonNull FlutterNativeView nativeView, @NonNull FlutterPluginRegistry pluginRegistry) {
    this.nativeView = nativeView;
    this.pluginRegistry = pluginRegistry;
  }

  //------- START PLUGINS ------
  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }
  //------- END PLUGINS -----

  //------- START ISOLATE METHOD CHANNEL COMMS ------
  @Override
  public void setMessageHandler(String channel, BinaryMessenger.BinaryMessageHandler handler) {
    // TODO: do we need to be attached for this?
    nativeView.setMessageHandler(channel, handler);
  }

  @Override
  public void send(String channel, ByteBuffer message) {
    send(channel, message, null);
  }

  @Override
  public void send(String channel, ByteBuffer message, BinaryMessenger.BinaryReply callback) {
    // TODO: ensure we're attached.
    nativeView.send(channel, message, callback);
  }
  //------- END ISOLATE METHOD CHANNEL COMMS -----

  //------ START JNI CALLS -----
  public native void nativeSurfaceCreated(long nativePlatformViewAndroid,
                                          Surface surface,
                                          int backgroundColor);

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
  //------ END JNI CALLS -----
}
