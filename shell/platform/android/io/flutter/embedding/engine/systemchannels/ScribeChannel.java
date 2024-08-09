// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.graphics.RectF;
import android.view.inputmethod.SelectGesture;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;
import java.util.HashMap;
import java.util.Arrays;

/**
 * {@link ScribeChannel} is a platform channel that is used by the framework to facilitate
 * the Scribe handwriting text input feature.
 */
public class ScribeChannel {
  private static final String TAG = "ScribeChannel";

  public final MethodChannel channel;
  private ScribeMethodHandler scribeMethodHandler;

  @NonNull
  public final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (scribeMethodHandler == null) {
            Log.v(
                TAG,
                "No ScribeMethodHandler registered, call not forwarded to spell check API.");
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          Log.v(TAG, "Received '" + method + "' message.");
          switch (method) {
            case "Scribe.startStylusHandwriting":
              try {
                scribeMethodHandler.startStylusHandwriting();
                result.success(null);
              } catch (IllegalStateException exception) {
                result.error("error", exception.getMessage(), null);
              }
              break;
            default:
              result.notImplemented();
              break;
          }
        }
      };

  public ScribeChannel(@NonNull DartExecutor dartExecutor) {
    channel = new MethodChannel(dartExecutor, "flutter/scribe", StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /**
   * Sets the {@link ScribeMethodHandler} which receives all requests for scribe
   * sent through this channel.
   */
  public void setScribeMethodHandler(
      @Nullable ScribeMethodHandler scribeMethodHandler) {
    this.scribeMethodHandler = scribeMethodHandler;
  }

  public interface ScribeMethodHandler {
    /**
     * Requests to start Scribe stylus handwriting, which will respond to the
     * {@code result} with either success if handwriting input has started or
     * error otherwise.
     */
    void startStylusHandwriting();
  }

  public void performHandwritingSelectGesture(SelectGesture gesture, MethodChannel.Result result) {
    System.out.println("justin sending performSelectionGesture for gesture: " + gesture);
    final HashMap<Object, Object> selectionAreaMap = new HashMap<>();
    final RectF selectionArea = gesture.getSelectionArea();
    selectionAreaMap.put("bottom", selectionArea.bottom);
    selectionAreaMap.put("top", selectionArea.top);
    selectionAreaMap.put("left", selectionArea.left);
    selectionAreaMap.put("right", selectionArea.right);
    // TODO(justinmc): Include granularity.
    final HashMap<Object, Object> gestureMap = new HashMap<>();
    gestureMap.put("selectionArea", selectionAreaMap);
    channel.invokeMethod("ScribeClient.performSelectionGesture", Arrays.asList(gestureMap), result);
  }
}
