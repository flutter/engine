package io.flutter.embedding.engine.dart;

import android.content.res.Resources;
import android.support.annotation.NonNull;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.view.FlutterRunArguments;

// TODO(mattcarroll): does it make sense to push PlatformMessageHandler into BinaryMessenger?
public class DartExecutor implements BinaryMessenger, PlatformMessageHandler {

  private static final String TAG = "DartExecutor";

  private final FlutterJNI flutterJNI;
  private final long nativeObjectReference;
  private final Resources resources;
  private final Map<String, BinaryMessenger.BinaryMessageHandler> messageHandlers;
  private final Map<Integer, BinaryMessenger.BinaryReply> mPendingReplies = new HashMap<>();
  private int mNextReplyId = 1;
  private boolean isApplicationRunning = false;
  private boolean isAttachedToJni = false;

  public DartExecutor(@NonNull FlutterJNI flutterJNI, long nativeObjectReference, @NonNull Resources resources) {
    this.flutterJNI = flutterJNI;
    this.nativeObjectReference = nativeObjectReference;
    this.resources = resources;
    this.messageHandlers = new HashMap<>();
  }

  public void onAttachedToJNI() {
    isAttachedToJni = true;
    flutterJNI.setPlatformMessageHandler(this);
  }

  public void onDetachedFromJNI() {
    isAttachedToJni = false;
    flutterJNI.setPlatformMessageHandler(null);
  }

  public boolean isApplicationRunning() {
    return isApplicationRunning;
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
    if (isApplicationRunning) {
      throw new AssertionError("This Flutter engine instance is already running an application");
    }

    flutterJNI.nativeRunBundleAndSnapshotFromLibrary(
        nativeObjectReference,
        bundlePath,
        defaultPath,
        entrypoint,
        libraryPath,
        resources.getAssets()
    );

    isApplicationRunning = true;
  }

  public void stop() {
    isApplicationRunning = false;
  }

  // TODO(mattcarroll): Implement observatory lookup for "flutter attach"
  public String getObservatoryUrl() {
    return flutterJNI.nativeGetObservatoryUri();
  }

  @Override
  public void setMessageHandler(String channel, BinaryMessenger.BinaryMessageHandler handler) {
    if (handler == null) {
      messageHandlers.remove(channel);
    } else {
      messageHandlers.put(channel, handler);
    }
  }

  @Override
  public void send(String channel, ByteBuffer message) {
    send(channel, message, null);
  }

  @Override
  public void send(String channel, ByteBuffer message, BinaryMessenger.BinaryReply callback) {
    if (!isAttachedToJni) {
      Log.d(TAG, "Send() called on a dart executor that is detached from JNI, channel=" + channel);
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
  @Override
  public void handlePlatformMessage(final String channel, byte[] message, final int replyId) {
    // TODO(mattcarroll): how can we possibly be detached from JNI when this method is invoked from JNI?
    assertAttachedToJni();
    BinaryMessenger.BinaryMessageHandler handler = messageHandlers.get(channel);
    if (handler != null) {
      try {
        final ByteBuffer buffer = (message == null ? null : ByteBuffer.wrap(message));
        handler.onMessage(buffer, new BinaryMessenger.BinaryReply() {
          private final AtomicBoolean done = new AtomicBoolean(false);
          @Override
          public void reply(ByteBuffer reply) {
            if (!isAttachedToJni) {
              Log.d(TAG,
                  "handlePlatformMessage replying to a dart executor that is detached from JNI, channel="
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

  @Override
  public void handlePlatformMessageResponse(int replyId, byte[] reply) {
    BinaryMessenger.BinaryReply callback = mPendingReplies.remove(replyId);
    if (callback != null) {
      try {
        callback.reply(reply == null ? null : ByteBuffer.wrap(reply));
      } catch (Exception ex) {
        Log.e(TAG, "Uncaught exception in binary message reply handler", ex);
      }
    }
  }

  private void assertAttachedToJni() {
    if (!isAttachedToJni) throw new AssertionError("Could not process message from platform because the dart executor is detached from JNI.");
  }
}
