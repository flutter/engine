// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;

/**
 * {@link SensitiveContentChannel} is a platform channel that is used by the framework to set the
 * content sensitivity of native Flutter Android {@code View}s.
 */
public class SensitiveContentChannel {
  private static final String TAG = "SensitiveContentChannel";

  public final MethodChannel channel;
  private SensitiveContentMethodHandler sensitiveContentMethodHandler;

  @NonNull
  public final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (sensitiveContentMethodHandler == null) {
            Log.v(
                TAG,
                "No SensitiveContentChannel registered, call not forwarded to sensitive content API.");
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case "SensitiveContent.setContentSensitivity":
              final int flutterViewId = argumentList.get(0);
              final int contentSensitivityLevel = argumentList.get(1);
              // TODO(camsim99): Add error handling if required.
              sensitiveContentMethodHandler.setContentSensitivity(
                  flutterViewId, contentSensitivityLevel);
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };

  public SensitiveContentChannel(@NonNull DartExecutor dartExecutor) {
    channel =
        new MethodChannel(dartExecutor, "flutter/sensitivecontent", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /**
   * Sets the {@link SensitiveContentMethodHandler} which receives all requests to set a particular
   * content sensitivty level sent through this channel.
   */
  public void setSensitiveContentMethodHandler(
      @Nullable SensitiveContentMethodHandler sensitiveContentMethodHandler) {
    this.sensitiveContentMethodHandler = sensitiveContentMethodHandler;
  }

  public interface SensitiveContentMethodHandler {
    /**
     * Requests that content being marked with the requested {@code contentSensitivity} mode for the
     * native Flutter Android {@code View} whose ID matches {@Long flutterViewId}.
     */
    void setContentSensitivity(
        @NonNull int flutterViewId,
        @NonNull int contentSensitivity,
        @NonNull MethodChannel.Result result);
  }
}
