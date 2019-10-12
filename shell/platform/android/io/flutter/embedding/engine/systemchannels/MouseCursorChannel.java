// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Iterator;

import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;

/**
 * TODO(dkwingsmt): fill in javadoc for MouseCursorChannel.
 */
public class MouseCursorChannel {
  private static final String TAG = "MouseCursorChannel";
  public static final String CHANNEL_NAME = "flutter/mousecursor";

  @NonNull
  public final MethodChannel channel;
  @Nullable
  private MouseCursorMethodHandler mouseCursorMethodHandler;

  public MouseCursorChannel(@NonNull DartExecutor dartExecutor) {
    channel = new MethodChannel(dartExecutor, CHANNEL_NAME, JSONMethodCodec.INSTANCE);
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
  @VisibleForTesting
  protected final MethodChannel.MethodCallHandler parsingMethodCallHandler = new MethodChannel.MethodCallHandler() {
    private void handleSetCursor(@NonNull JSONArray argumentList, @NonNull MethodChannel.Result result) {
      int cursor;
      try {
        // Android only supports one pointer device
        final JSONObject deviceCursorsRaw = argumentList.getJSONObject(0);
        if (deviceCursorsRaw.length() != 1) {
          throw new Exception(String.format("Expect one device request but received %d", deviceCursorsRaw.length()));
        }
        final String key = deviceCursorsRaw.keys().next();
        cursor = deviceCursorsRaw.getInt(key);
      } catch (Exception e) {
        result.error("error", "Parameter error: " + e.getMessage(), null);
        return;
      }
      try {
        mouseCursorMethodHandler.setCursor(cursor);
      } catch (Exception e) {
        result.error("error", "Error when setting cursors: " + e.getMessage(), null);
        return;
      }
      result.success(true);
    }

    @Override
    public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
      if (mouseCursorMethodHandler == null) {
        // If no explicit mouseCursorMethodHandler  has been registered then we don't
        // need to forward this call to an API. Return.
        return;
      }

      final String method = call.method;
      final JSONArray argumentList = (JSONArray) call.arguments;
      Log.v(TAG, "Received '" + method + "' message.");
      try {
        switch (method) {
          case "setCursors":
            handleSetCursor(argumentList, result);
            break;
          default:
        }
      } catch (Exception e) {
        result.error("error", "Unhandled error: " + e.getMessage(), null);
      }
    }
  };

  public interface MouseCursorMethodHandler {
    // TODO(dkwingsmt): javadoc
    void setCursor(@NonNull int cursor);
  }


}
