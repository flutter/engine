package io.flutter.embedding;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import io.flutter.embedding.legacy.FlutterPluginRegistry;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.view.FlutterRunArguments;

public class FlutterEngine implements BinaryMessenger {
  private static final String TAG = "FlutterEngine";

  private long nativeObjectReference;
  private FlutterJNI flutterJNI;
  private FlutterRenderer renderer;
  private FlutterPluginRegistry pluginRegistry;
  private boolean isBackgroundView; // TODO(mattcarroll): rename to something without "view"

  FlutterEngine(
      Context context,
      Resources resources,
      boolean isBackgroundView
  ) {
    this.flutterJNI = new FlutterJNI();
    this.resources = resources;
    this.isBackgroundView = isBackgroundView;

    pluginRegistry = new FlutterPluginRegistry(this, context);
    attach();
    assertAttached();
    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling.
    this.renderer = new FlutterRenderer(this, flutterJNI, nativeObjectReference);
    mMessageHandlers = new HashMap<>();
  }

  public FlutterRenderer getRenderer() {
    return renderer;
  }

  //------- START PLUGINS ------
  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }
  //------- END PLUGINS -----

  //------- START ISOLATE METHOD CHANNEL COMMS ------
  @Override
  public void setMessageHandler(String channel, BinaryMessageHandler handler) {
    if (handler == null) {
      mMessageHandlers.remove(channel);
    } else {
      mMessageHandlers.put(channel, handler);
    }
  }

  @Override
  public void send(String channel, ByteBuffer message) {
    send(channel, message, null);
  }
  //------- END ISOLATE METHOD CHANNEL COMMS -----

  //------- START COPY FROM FlutterNativeView -----
  private final Map<String, BinaryMessageHandler> mMessageHandlers;
  private int mNextReplyId = 1;
  private final Map<Integer, BinaryReply> mPendingReplies = new HashMap<>();

  private final Resources resources;
  private boolean applicationIsRunning;

  private void attach() {
    nativeObjectReference = flutterJNI.nativeAttach(this, isBackgroundView);
  }

  public void detach() {
    pluginRegistry.detach();
    flutterJNI.nativeDetach(nativeObjectReference);
  }

  public void destroy() {
    pluginRegistry.destroy();
    flutterJNI.nativeDestroy(nativeObjectReference);
    nativeObjectReference = 0;
    applicationIsRunning = false;
  }

  public void attachViewAndActivity(Activity activity) {
    pluginRegistry.attach(this, activity);
  }

  public boolean isAttached() {
    return nativeObjectReference != 0;
  }

  public long get() {
    return nativeObjectReference;
  }

  public void assertAttached() {
    if (!isAttached()) throw new AssertionError("Platform view is not attached");
  }

  public void runFromBundle(FlutterRunArguments args) {
    if (args.bundlePath == null) {
      throw new AssertionError("A bundlePath must be specified");
    } else if (args.entrypoint == null) {
      throw new AssertionError("An entrypoint must be specified");
    }
    runFromBundleInternal(args.bundlePath, args.entrypoint, args.libraryPath, args.defaultPath);
  }

  /**
   * @deprecated
   * Please use runFromBundle with `FlutterRunArguments`.
   * Parameter `reuseRuntimeController` has no effect.
   */
  @Deprecated
  public void runFromBundle(String bundlePath, String defaultPath, String entrypoint,
                            boolean reuseRuntimeController) {
    runFromBundleInternal(bundlePath, entrypoint, null, defaultPath);
  }

  private void runFromBundleInternal(String bundlePath, String entrypoint,
                                     String libraryPath, String defaultPath) {
    assertAttached();
    if (applicationIsRunning)
      throw new AssertionError(
          "This Flutter engine instance is already running an application");
    flutterJNI.nativeRunBundleAndSnapshotFromLibrary(nativeObjectReference, bundlePath,
        defaultPath, entrypoint, libraryPath, resources.getAssets());

    applicationIsRunning = true;
  }

  public boolean isApplicationRunning() {
    return applicationIsRunning;
  }

  public String getObservatoryUri() {
    return flutterJNI.nativeGetObservatoryUri();
  }

  @Override
  public void send(String channel, ByteBuffer message, BinaryReply callback) {
    if (!isAttached()) {
      Log.d(TAG, "FlutterView.send called on a detached view, channel=" + channel);
      return;
    }

    int replyId = 0;
    if (callback != null) {
      replyId = mNextReplyId++;
      mPendingReplies.put(replyId, callback);
    }
    if (message == null) {
      flutterJNI.nativeDispatchEmptyPlatformMessage(nativeObjectReference, channel, replyId);
    } else {
      flutterJNI.nativeDispatchPlatformMessage(
          nativeObjectReference, channel, message, message.position(), replyId);
    }
  }

  // Called by native to send us a platform message.
  @SuppressWarnings("unused")
  private void handlePlatformMessage(final String channel, byte[] message, final int replyId) {
    assertAttached();
    BinaryMessageHandler handler = mMessageHandlers.get(channel);
    if (handler != null) {
      try {
        final ByteBuffer buffer = (message == null ? null : ByteBuffer.wrap(message));
        handler.onMessage(buffer, new BinaryReply() {
          private final AtomicBoolean done = new AtomicBoolean(false);
          @Override
          public void reply(ByteBuffer reply) {
            if (!isAttached()) {
              Log.d(TAG,
                  "handlePlatformMessage replying to a detached view, channel="
                      + channel);
              return;
            }
            if (done.getAndSet(true)) {
              throw new IllegalStateException("Reply already submitted");
            }
            if (reply == null) {
              flutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(nativeObjectReference, replyId);
            } else {
              flutterJNI.nativeInvokePlatformMessageResponseCallback(nativeObjectReference, replyId, reply, reply.position());
            }
          }
        });
      } catch (Exception ex) {
        Log.e(TAG, "Uncaught exception in binary message listener", ex);
        flutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(nativeObjectReference, replyId);
      }
      return;
    }
    flutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(nativeObjectReference, replyId);
  }

  // Called by native to respond to a platform message that we sent.
  @SuppressWarnings("unused")
  private void handlePlatformMessageResponse(int replyId, byte[] reply) {
    BinaryReply callback = mPendingReplies.remove(replyId);
    if (callback != null) {
      try {
        callback.reply(reply == null ? null : ByteBuffer.wrap(reply));
      } catch (Exception ex) {
        Log.e(TAG, "Uncaught exception in binary message reply handler", ex);
      }
    }
  }

  // Called by native to update the semantics/accessibility tree.
  @SuppressWarnings("unused")
  private void updateSemantics(ByteBuffer buffer, String[] strings) {
    Log.d(TAG, "updateSemantics()");
    renderer.updateSemantics(buffer, strings);
  }

  // Called by native to update the custom accessibility actions.
  @SuppressWarnings("unused")
  private void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
    Log.d(TAG, "updateCustomAccessibilityActions()");
    renderer.updateCustomAccessibilityActions(buffer, strings);
  }

  // Called by native to notify first Flutter frame rendered.
  @SuppressWarnings("unused")
  private void onFirstFrame() {
    Log.d(TAG, "onFirstFrame()");
    renderer.onFirstFrameRendered();
  }

  // Called by native to notify when the engine is restarted (cold reload).
  @SuppressWarnings("unused")
  private void onPreEngineRestart() {
    if (pluginRegistry == null)
      return;
    pluginRegistry.onPreEngineRestart();
  }
  //------- END COPY FROM FlutterNativeView ----
}
