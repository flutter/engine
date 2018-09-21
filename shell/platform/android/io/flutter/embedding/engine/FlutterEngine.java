package io.flutter.embedding.engine;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.legacy.FlutterPluginRegistry;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.view.FlutterRunArguments;

/**
 * A single Flutter execution environment.
 *
 * A {@code FlutterEngine} can execute in the background, or it can be rendered to the screen by
 * using the accompanying {@link FlutterRenderer}.  Rendering can be started and stopped, thus
 * allowing a {@code FlutterEngine} to move from UI interaction to data-only processing and then
 * back to UI interaction.
 *
 * To start running Flutter within this {@code FlutterEngine}, use {@link #runFromBundle(FlutterRunArguments)}.
 * The {@link #runFromBundle(FlutterRunArguments)} method must not be invoked twice on the same
 * {@code FlutterEngine}.
 *
 * To start rendering Flutter content to the screen, use {@link #getRenderer()} to obtain a
 * {@link FlutterRenderer} and then attach a {@link FlutterRenderer.RenderSurface}.  Consider using
 * a {@link io.flutter.embedding.android.FlutterView} as a {@link FlutterRenderer.RenderSurface}.
 */
public class FlutterEngine implements BinaryMessenger {
  private static final String TAG = "FlutterEngine";

  private final Resources resources;
  private final FlutterJNI flutterJNI;
  private final FlutterRenderer renderer;
  private final FlutterPluginRegistry pluginRegistry;
  private final Map<String, BinaryMessageHandler> mMessageHandlers;
  private final Map<Integer, BinaryReply> mPendingReplies = new HashMap<>();
  private long nativeObjectReference;
  private boolean isBackgroundView; // TODO(mattcarroll): rename to something without "view"
  private boolean applicationIsRunning;
  private int mNextReplyId = 1;

  public FlutterEngine(
      Context context,
      Resources resources,
      boolean isBackgroundView
  ) {
    this.flutterJNI = new FlutterJNI();
    this.resources = resources;
    this.isBackgroundView = isBackgroundView;
    pluginRegistry = new FlutterPluginRegistry(this, context);
    mMessageHandlers = new HashMap<>();

    attachToJni();
    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling if possible.
    this.renderer = new FlutterRenderer(flutterJNI, nativeObjectReference);
  }

  private void attachToJni() {
    // TODO(mattcarroll): what impact does "isBackgroundView' have?
    nativeObjectReference = flutterJNI.nativeAttach(this, isBackgroundView);

    if (!isAttachedToJni()) {
      throw new RuntimeException("FlutterEngine failed to attach to its native Object reference.");
    }
  }

  private void assertAttachedToJni() {
    if (!isAttachedToJni()) throw new AssertionError("Platform view is not attached");
  }

  @SuppressWarnings("BooleanMethodIsAlwaysInverted")
  private boolean isAttachedToJni() {
    return nativeObjectReference != 0;
  }

  public void detachFromJni() {
    pluginRegistry.detach();
    // TODO(mattcarroll): why do we have a nativeDetach() method? can we get rid of this?
    flutterJNI.nativeDetach(nativeObjectReference);
  }

  public void destroy() {
    pluginRegistry.destroy();
    flutterJNI.nativeDestroy(nativeObjectReference);
    nativeObjectReference = 0;
    applicationIsRunning = false;
  }

  public void runFromBundle(FlutterRunArguments args) {
    if (args.bundlePath == null) {
      throw new AssertionError("A bundlePath must be specified");
    } else if (args.entrypoint == null) {
      throw new AssertionError("An entrypoint must be specified");
    }
    runFromBundleInternal(args.bundlePath, args.entrypoint, args.libraryPath, args.defaultPath);
  }

  private void runFromBundleInternal(
      String bundlePath,
      String entrypoint,
      String libraryPath,
      String defaultPath
  ) {
    assertAttachedToJni();

    if (applicationIsRunning) {
      throw new AssertionError("This Flutter engine instance is already running an application");
    }

    flutterJNI.nativeRunBundleAndSnapshotFromLibrary(nativeObjectReference, bundlePath,
        defaultPath, entrypoint, libraryPath, resources.getAssets());

    applicationIsRunning = true;
  }

  public boolean isApplicationRunning() {
    return applicationIsRunning;
  }

  // TODO(mattcarroll): what does this callback actually represent?
  // Called by native to notify when the engine is restarted (cold reload).
  @SuppressWarnings("unused")
  private void onPreEngineRestart() {
    if (pluginRegistry == null)
      return;
    pluginRegistry.onPreEngineRestart();
  }

  public FlutterRenderer getRenderer() {
    return renderer;
  }

  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }

  // TODO(mattcarroll): Is this method really all about plugins? or does it have other implications?
  public void attachViewAndActivity(Activity activity) {
    pluginRegistry.attach(this, activity);
  }

  // TODO(mattcarroll): Implement observatory lookup for "flutter attach"
  public String getObservatoryUrl() {
    return flutterJNI.nativeGetObservatoryUri();
  }

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

  @Override
  public void send(String channel, ByteBuffer message, BinaryReply callback) {
    if (!isAttachedToJni()) {
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
    assertAttachedToJni();
    BinaryMessageHandler handler = mMessageHandlers.get(channel);
    if (handler != null) {
      try {
        final ByteBuffer buffer = (message == null ? null : ByteBuffer.wrap(message));
        handler.onMessage(buffer, new BinaryReply() {
          private final AtomicBoolean done = new AtomicBoolean(false);
          @Override
          public void reply(ByteBuffer reply) {
            if (!isAttachedToJni()) {
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
  //------- END ISOLATE METHOD CHANNEL COMMS -----

  //------ START NATIVE CALLBACKS THAT SHOULD BE MOVED TO FlutterRenderer ------
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
  //------ END NATIVE CALLBACKS THAT SHOULD BE MOVED TO FlutterRenderer ------
}
