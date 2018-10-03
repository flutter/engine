// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import io.flutter.app.FlutterPluginRegistry;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.renderer.FlutterRenderer.RenderSurface;
import io.flutter.plugin.common.*;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.HashMap;
import java.util.Map;
import android.content.res.AssetManager;
import io.flutter.embedding.engine.dart.PlatformMessageHandler;

public class FlutterNativeView implements BinaryMessenger {
    private static final String TAG = "FlutterNativeView";

    private final Map<String, BinaryMessageHandler> mMessageHandlers;
    private int mNextReplyId = 1;
    private final Map<Integer, BinaryReply> mPendingReplies = new HashMap<>();

    private final FlutterPluginRegistry mPluginRegistry;
    private long mNativePlatformView;
    private FlutterView mFlutterView;
    private FlutterJNI mFlutterJNI;
    private final PlatformMessageHandlerImpl mPlatformMessageHandler;
    private final RenderSurfaceImpl mRenderSurface;
    private final Context mContext;
    private boolean applicationIsRunning;

    public FlutterNativeView(Context context) {
        this(context, false);
    }

    public FlutterNativeView(Context context, boolean isBackgroundView) {
        mContext = context;
        mPluginRegistry = new FlutterPluginRegistry(this, context);
        mFlutterJNI = new FlutterJNI();
        mRenderSurface = new RenderSurfaceImpl();
        mFlutterJNI.setRenderSurface(mRenderSurface);
        mPlatformMessageHandler = new PlatformMessageHandlerImpl();
        mFlutterJNI.setPlatformMessageHandler(mPlatformMessageHandler);
        attach(this, isBackgroundView);
        assertAttached();
        mMessageHandlers = new HashMap<>();
    }

    public void detach() {
        mPluginRegistry.detach();
        mFlutterView = null;
        mFlutterJNI.nativeDetach(mNativePlatformView);
    }

    public void destroy() {
        mPluginRegistry.destroy();
        mFlutterView = null;
        mFlutterJNI.nativeDestroy(mNativePlatformView);
        mNativePlatformView = 0;
        applicationIsRunning = false;
    }

    public FlutterPluginRegistry getPluginRegistry() {
        return mPluginRegistry;
    }

    public void attachViewAndActivity(FlutterView flutterView, Activity activity) {
        mFlutterView = flutterView;
        mPluginRegistry.attach(flutterView, activity);
    }

    public boolean isAttached() {
        return mNativePlatformView != 0;
    }

    public long get() {
        return mNativePlatformView;
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
        mFlutterJNI.nativeRunBundleAndSnapshotFromLibrary(mNativePlatformView, bundlePath,
            defaultPath, entrypoint, libraryPath, mContext.getResources().getAssets());

        applicationIsRunning = true;
    }

    public boolean isApplicationRunning() {
        return applicationIsRunning;
    }

    public static String getObservatoryUri() {
        FlutterJNI flutterJNI = new FlutterJNI();
        return flutterJNI.nativeGetObservatoryUri();
    }

    @Override
    public void send(String channel, ByteBuffer message) {
        send(channel, message, null);
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
            mFlutterJNI.nativeDispatchEmptyPlatformMessage(mNativePlatformView, channel, replyId);
        } else {
            mFlutterJNI.nativeDispatchPlatformMessage(
                    mNativePlatformView, channel, message, message.position(), replyId);
        }
    }

    @Override
    public void setMessageHandler(String channel, BinaryMessageHandler handler) {
        if (handler == null) {
            mMessageHandlers.remove(channel);
        } else {
            mMessageHandlers.put(channel, handler);
        }
    }

    /*package*/ FlutterJNI getFlutterJNI() {
        return mFlutterJNI;
    }

    private void attach(FlutterNativeView view, boolean isBackgroundView) {
        mNativePlatformView = mFlutterJNI.nativeAttach(mFlutterJNI, isBackgroundView);
    }

    private final class PlatformMessageHandlerImpl implements PlatformMessageHandler {
        // Called by native to send us a platform message.
        public void handlePlatformMessage(final String channel, byte[] message, final int replyId) {
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
                                mFlutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(
                                        mNativePlatformView, replyId);
                            } else {
                                mFlutterJNI.nativeInvokePlatformMessageResponseCallback(
                                        mNativePlatformView, replyId, reply, reply.position());
                            }
                        }
                    });
                } catch (Exception ex) {
                    Log.e(TAG, "Uncaught exception in binary message listener", ex);
                    mFlutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(mNativePlatformView, replyId);
                }
                return;
            }
            mFlutterJNI.nativeInvokePlatformMessageEmptyResponseCallback(mNativePlatformView, replyId);
        }

        // Called by native to respond to a platform message that we sent.
        public void handlePlatformMessageResponse(int replyId, byte[] reply) {
            BinaryReply callback = mPendingReplies.remove(replyId);
            if (callback != null) {
                try {
                    callback.reply(reply == null ? null : ByteBuffer.wrap(reply));
                } catch (Exception ex) {
                    Log.e(TAG, "Uncaught exception in binary message reply handler", ex);
                }
            }
        }
    }

    private final class RenderSurfaceImpl implements RenderSurface {
        // Called by native to update the semantics/accessibility tree.
        public void updateSemantics(ByteBuffer buffer, String[] strings) {
            if (mFlutterView == null) return;
            mFlutterView.updateSemantics(buffer, strings);
        }

        // Called by native to update the custom accessibility actions.
        public void updateCustomAccessibilityActions(ByteBuffer buffer, String[] strings) {
            if (mFlutterView == null)
                return;
            mFlutterView.updateCustomAccessibilityActions(buffer, strings);
        }

        // Called by native to notify first Flutter frame rendered.
        public void onFirstFrameRendered() {
            if (mFlutterView == null) return;
            mFlutterView.onFirstFrame();
        }

        /*package*/ FlutterJNI getFlutterJNI() {
            return mFlutterJNI;
        }
    }

    // Called by native to notify when the engine is restarted (cold reload).
    @SuppressWarnings("unused")
    private void onPreEngineRestart() {
        if (mPluginRegistry == null)
            return;
        mPluginRegistry.onPreEngineRestart();
    }
}
