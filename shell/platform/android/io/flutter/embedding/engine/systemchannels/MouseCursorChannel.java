// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import org.json.JSONArray;

import java.util.HashMap;

import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;

/**
 * System channel that receives requests for mouse cursor behavior, e.g., set as system cursors.
 */
public class MouseCursorChannel {
  private static final String TAG = "MouseCursorChannel";
  public static final String CHANNEL_NAME = "flutter/mousecursor";

  @NonNull
  public final MethodChannel channel;
  @Nullable
  private MouseCursorMethodHandler mouseCursorMethodHandler;

  public MouseCursorChannel(@NonNull DartExecutor dartExecutor) {
    channel = new MethodChannel(dartExecutor, CHANNEL_NAME, StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodCallHandler);
  }

  /**
   * Sets the {@link MouseCursorMethodHandler} which receives all events and requests
   * that are parsed from the underlying platform channel.
   */
  public void setMethodHandler(@Nullable MouseCursorMethodHandler mouseCursorMethodHandler) {
    this.mouseCursorMethodHandler = mouseCursorMethodHandler;
  }

  @NonNull
  private final MethodChannel.MethodCallHandler parsingMethodCallHandler = new MethodChannel.MethodCallHandler() {
    @Override
    public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
      if (mouseCursorMethodHandler == null) {
        // If no explicit mouseCursorMethodHandler has been registered then we don't
        // need to forward this call to an API. Return.
        return;
      }

      final String method = call.method;
      Log.v(TAG, "Received '" + method + "' message.");
      try {
        switch (method) {
          case "activateSystemCursor":
            @SuppressWarnings("unchecked")
            final HashMap<String, Object> data = (HashMap<String, Object>) call.arguments;
            final Integer shapeCode = (Integer) data.get("shapeCode");
            try {
              mouseCursorMethodHandler.activateSystemCursor(shapeCode);
            } catch (Exception e) {
              result.error("error", "Error when setting cursors: " + e.getMessage(), null);
              break;
            }
            result.success(true);
            break;
          default:
        }
      } catch (Exception e) {
        result.error("error", "Unhandled error: " + e.getMessage(), null);
      }
    }
  };

  public interface MouseCursorMethodHandler {
    // Called when the pointer should start displaying a system mouse cursor
    // specified by {@code shapeCode}.
    public void activateSystemCursor(@NonNull Integer shapeCode);
  }


}