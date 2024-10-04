// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import static io.flutter.Build.API_LEVELS;

import android.annotation.TargetApi;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.VisibleForTesting;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;

/**
 * {@link ScribeChannel} is a platform channel that is used by the framework to facilitate the
 * Scribe handwriting text input feature.
 */
public class ScribeChannel {
  private static final String TAG = "ScribeChannel";

  @VisibleForTesting
  public static final String METHOD_IS_STYLUS_HANDWRITING_AVAILABLE =
      "Scribe.isStylusHandwritingAvailable";

  @VisibleForTesting
  public static final String METHOD_START_STYLUS_HANDWRITING = "Scribe.startStylusHandwriting";

  public final MethodChannel channel;
  private ScribeMethodHandler scribeMethodHandler;

  @NonNull
  public final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (scribeMethodHandler == null) {
            Log.v(TAG, "No ScribeMethodHandler registered. Scribe call not handled.");
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case METHOD_IS_STYLUS_HANDWRITING_AVAILABLE:
              isStylusHandwritingAvailable(call, result);
              break;
            case METHOD_START_STYLUS_HANDWRITING:
              startStylusHandwriting(call, result);
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };

  private void isStylusHandwritingAvailable(
      @NonNull MethodCall call, @NonNull MethodChannel.Result result) {
    try {
      final boolean isAvailable = scribeMethodHandler.isStylusHandwritingAvailable();
      result.success(isAvailable);
    } catch (IllegalStateException exception) {
      result.error("error", exception.getMessage(), null);
    }
  }

  private void startStylusHandwriting(
      @NonNull MethodCall call, @NonNull MethodChannel.Result result) {
    try {
      scribeMethodHandler.startStylusHandwriting();
      result.success(null);
    } catch (IllegalStateException exception) {
      result.error("error", exception.getMessage(), null);
    }
  }

  public ScribeChannel(@NonNull DartExecutor dartExecutor) {
    channel = new MethodChannel(dartExecutor, "flutter/scribe", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /**
   * Sets the {@link ScribeMethodHandler} which receives all requests for scribe sent through this
   * channel.
   */
  public void setScribeMethodHandler(@Nullable ScribeMethodHandler scribeMethodHandler) {
    this.scribeMethodHandler = scribeMethodHandler;
  }

  public interface ScribeMethodHandler {
    /**
     * Responds to the {@code result} with success and a boolean indicating whether or not stylus
     * hadnwriting is available.
     */
    @TargetApi(API_LEVELS.API_34)
    @RequiresApi(API_LEVELS.API_34)
    boolean isStylusHandwritingAvailable();

    /**
     * Requests to start Scribe stylus handwriting, which will respond to the {@code result} with
     * either success if handwriting input has started or error otherwise.
     */
    @TargetApi(API_LEVELS.API_33)
    @RequiresApi(API_LEVELS.API_33)
    void startStylusHandwriting();
  }

  // TODO(justinmc): Scribe stylus gestures should be supported here.
  // https://github.com/flutter/flutter/issues/156018
}
