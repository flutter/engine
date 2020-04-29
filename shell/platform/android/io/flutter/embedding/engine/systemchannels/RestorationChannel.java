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
 */
public class RestorationChannel {
  private static final String TAG = "RestorationChannel";

  public RestorationChannel(@NonNull DartExecutor dartExecutor) {
    MethodChannel channel =
        new MethodChannel(dartExecutor, "flutter/restoration", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(handler);
  }

  private byte[] dataForFramework;
  private byte[] dataFromFramework;

  /** Obtain the most current restoration data that the framework has provided. */
  public byte[] getRestorationDataFromFramework() {
    return dataFromFramework;
  }

  /** Set the restoration data that will be sent to the framework when the framework requests it. */
  public void setRestorationDataForFramework(byte[] data) {
    dataForFramework = data;
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
              dataFromFramework = (byte[]) args;
              result.success(null);
              break;
            case "get":
              result.success(dataForFramework);
              dataForFramework = null;
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };
}
