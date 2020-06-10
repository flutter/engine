// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;

/**
 * System channel to exchange restoration data between framework and engine.
 *
 * <p>The engine can obtain the current restoration data from the framework via this channel to
 * store it on disk and - when the app is relaunched - provide the stored data back to the framework
 * to recreate the original state of the app.
 *
 * <p>The channel can be configured to delay responding to the framework's request for restoration
 * data via {@code waitForRestorationData} until the engine-side has provided the data. This is
 * useful for use cases where the engine is pre-warmed at a point in the application's life cycle
 * where the operating system has not been made available to the engine yet. For example, if the
 * engine is pre-warmed as part of the Application before an Activity is created, this flag should
 * be set to true because Android will only provide the restoration data to the Activity.
 *
 * <p>The current restoration data provided by the framework can be read via {@code
 * getRestorationData()}.
 */
public class RestorationChannel {
  private static final String TAG = "RestorationChannel";

  public RestorationChannel(
      @NonNull DartExecutor dartExecutor, @NonNull boolean waitForRestorationData) {
    this(
        new MethodChannel(dartExecutor, "flutter/restoration", StandardMethodCodec.INSTANCE),
        waitForRestorationData);
  }

  RestorationChannel(MethodChannel channel, @NonNull boolean waitForRestorationData) {
    this.channel = channel;
    this.waitForRestorationData = waitForRestorationData;

    channel.setMethodCallHandler(handler);
  }

  /**
   * Whether {@code setRestorationData} will be called to provide restoration data for the
   * framework.
   *
   * <p>When this is set to true, the channel will delay answering any requests for restoration data
   * by the framework until {@code setRestorationData} has been called. It must be set to false if
   * the engine never calls {@code setRestorationData}. If it has been set to true, but it later
   * turns out that there is no restoration data, {@code setRestorationData} must be called with
   * null.
   */
  public final boolean waitForRestorationData;

  private MethodChannel channel;
  private byte[] restorationData;
  private MethodChannel.Result pendingResult;
  private boolean engineHasProvidedData = false;
  private boolean frameworkHasRequestedData = false;

  /** Obtain the most current restoration data that the framework has provided. */
  public byte[] getRestorationData() {
    return restorationData;
  }

  /** Set the restoration data from which the framework will restore its state. */
  public void setRestorationData(byte[] data) {
    engineHasProvidedData = true;
    if (pendingResult != null) {
      pendingResult.success(data);
      pendingResult = null;
      restorationData = data;
    } else if (frameworkHasRequestedData) {
      channel.invokeMethod(
          "push",
          data,
          new MethodChannel.Result() {
            @Override
            public void success(Object result) {
              restorationData = data;
            }

            @Override
            public void error(String errorCode, String errorMessage, Object errorDetails) {
              Log.e(
                  TAG,
                  "Error "
                      + errorCode
                      + " while sending restoration data to framework: "
                      + errorMessage);
            }

            @Override
            public void notImplemented() {
              // Nothing to do.
            }
          });
    } else {
      restorationData = data;
    }
  }

  /**
   * Clears the current restoration data.
   *
   * <p>This should be called just prior to a hot restart. Otherwise, after the hot restart the
   * state prior to the hot restart will get restored.
   */
  public void clearData() {
    restorationData = null;
  }

  private final MethodChannel.MethodCallHandler handler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          final String method = call.method;
          final Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case "put":
              restorationData = (byte[]) args;
              result.success(null);
              break;
            case "get":
              frameworkHasRequestedData = true;
              if (engineHasProvidedData || !waitForRestorationData) {
                result.success(restorationData);
              } else {
                pendingResult = result;
              }
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };
}
