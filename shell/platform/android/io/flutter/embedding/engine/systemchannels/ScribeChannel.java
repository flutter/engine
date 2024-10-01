// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.graphics.RectF;
import android.os.CancellationSignal;
import android.view.inputmethod.DeleteGesture;
import android.view.inputmethod.DeleteRangeGesture;
import android.view.inputmethod.HandwritingGesture;
import android.view.inputmethod.PreviewableHandwritingGesture;
import android.view.inputmethod.SelectGesture;
import android.view.inputmethod.SelectRangeGesture;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;
import java.util.HashMap;

/**
 * {@link ScribeChannel} is a platform channel that is used by the framework to facilitate the
 * Scribe handwriting text input feature.
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
            Log.v(TAG, "No ScribeMethodHandler registered, call not forwarded to spell check API.");
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
   * Sets the {@link ScribeMethodHandler} which receives all requests for scribe sent through this
   * channel.
   */
  public void setScribeMethodHandler(@Nullable ScribeMethodHandler scribeMethodHandler) {
    this.scribeMethodHandler = scribeMethodHandler;
  }

  public interface ScribeMethodHandler {
    /**
     * Requests to start Scribe stylus handwriting, which will respond to the {@code result} with
     * either success if handwriting input has started or error otherwise.
     */
    void startStylusHandwriting();
  }

  public void previewHandwritingGesture(PreviewableHandwritingGesture gesture, CancellationSignal cancellationSignal) {
    System.out.println("justin sending previewHandwritingGesture for gesture: " + gesture);
    final HashMap<Object, Object> gestureMap = new HashMap<>();
    if (gesture instanceof DeleteGesture) {
      gestureMap.put("type", "delete");
    } else if (gesture instanceof DeleteRangeGesture) {
      gestureMap.put("type", "deleteRange");
    } else if (gesture instanceof SelectGesture) {
      gestureMap.put("type", "select");
    } else if (gesture instanceof SelectRangeGesture) {
      gestureMap.put("type", "selectRange");
    } else {
      return;
    }

    // TODO(justinmc): You'll need to provide some kind of API that allows users
    // to cancel a previewed gesture. Maybe keep ahold of cancellationSignal
    // here, then provide platform channel methods for cancel, isCanceled,
    // setOnCancelListener, and throwIfCanceled.
    channel.invokeMethod("ScribeClient.previewHandwritingGesture", gestureMap);
  }

  public void performHandwritingGesture(HandwritingGesture gesture, MethodChannel.Result result) {
    System.out.println("justin sending performHandwritingGesture for gesture: " + gesture);

    final HashMap<Object, Object> gestureMap = new HashMap<>();
    if (gesture instanceof SelectGesture) {
      final SelectGesture selectGesture = (SelectGesture) gesture;
      final HashMap<Object, Object> selectionAreaMap = new HashMap<>();
      final RectF selectionArea = selectGesture.getSelectionArea();
      selectionAreaMap.put("bottom", selectionArea.bottom);
      selectionAreaMap.put("top", selectionArea.top);
      selectionAreaMap.put("left", selectionArea.left);
      selectionAreaMap.put("right", selectionArea.right);
      gestureMap.put("type", "select");
      gestureMap.put("granularity", selectGesture.getGranularity());
      gestureMap.put("selectionArea", selectionAreaMap);
    } else if (gesture instanceof DeleteGesture) {
      final DeleteGesture deleteGesture = (DeleteGesture) gesture;
      final HashMap<Object, Object> deletionAreaMap = new HashMap<>();
      final RectF deletionArea = deleteGesture.getDeletionArea();
      deletionAreaMap.put("bottom", deletionArea.bottom);
      deletionAreaMap.put("top", deletionArea.top);
      deletionAreaMap.put("left", deletionArea.left);
      deletionAreaMap.put("right", deletionArea.right);
      gestureMap.put("type", "delete");
      gestureMap.put("granularity", deleteGesture.getGranularity());
      gestureMap.put("deletionArea", deletionAreaMap);
    }
    // TODO(justinmc): All other gestures. https://developer.android.com/reference/android/view/inputmethod/HandwritingGesture#public-methods

    channel.invokeMethod("ScribeClient.performHandwritingGesture", gestureMap, result);
  }
}
