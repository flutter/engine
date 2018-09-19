package io.flutter.embedding;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import java.nio.ByteBuffer;

public class FlutterJNI {
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
  public native long nativeAttach(FlutterEngine engine, boolean isBackgroundView);
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
}
