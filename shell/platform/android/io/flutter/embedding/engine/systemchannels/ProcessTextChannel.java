// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.systemchannels;

import android.content.pm.PackageManager;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.StandardMethodCodec;
import java.util.ArrayList;
import java.util.Map;

/**
 * {@link ProcessTextChannel} is a platform channel that is used by the framework to initiate text
 * processing feature in the embedding and for the embedding to send back the results.
 *
 * <p>When there is new a text context menu to display, the framework will send to the embedding the
 * message {@code ProcessText.queryTextActions}. In response, the {@link
 * io.flutter.plugin.text.ProcessTextPlugin} will return a map of all activities that can be performed
 * to process text. The map keys are generated IDs and the values are the activities labels.
 * On the first request, the {@link io.flutter.plugin.text.ProcessTextPlugin} will make a call to
 * Android's package manager to query all activities that can be performed for the
 * {@code Intent.ACTION_PROCESS_TEXT} intent.
 *
 * <p>When an text processing action has to be executed, the framework will send to the embedding the
 * message {@code ProcessText.processTextAction} with the {@code int id} of the choosen text action and the
 * {@code String} of text to process as arguments. In response, the {@link
 * io.flutter.plugin.text.ProcessTextPlugin} will make a call to the Android application activity to
 * start the activity exposing the text action and it will return the processed text, or null if the
 * activity did not return a value.
 *
 * <p>{@link io.flutter.plugin.text.ProcessTextPlugin} implements {@link ProcessTextMethodHandler}
 * that parses incoming messages from Flutter.
 */
public class ProcessTextChannel {
  private static final String TAG = "ProcessTextChannel";
  private static final String CHANNEL_NAME = "flutter/processtext";
  private static final String METHOD_QUERY_TEXT_ACTIONS = "ProcessText.queryTextActions";
  private static final String METHOD_PROCESS_TEXT_ACTION = "ProcessText.processTextAction";

  public final MethodChannel channel;
  public final PackageManager packageManager;
  private ProcessTextMethodHandler processTextMethodHandler;

  @NonNull
  public final MethodChannel.MethodCallHandler parsingMethodHandler =
      new MethodChannel.MethodCallHandler() {
        @Override
        public void onMethodCall(@NonNull MethodCall call, @NonNull MethodChannel.Result result) {
          if (processTextMethodHandler == null) {
            return;
          }
          String method = call.method;
          Object args = call.arguments;
          switch (method) {
            case METHOD_QUERY_TEXT_ACTIONS:
              try {
                Map<Integer, String> actions = processTextMethodHandler.queryTextActions();
                result.success(actions);
              } catch (IllegalStateException exception) {
                result.error("error", exception.getMessage(), null);
              }
              break;
            case METHOD_PROCESS_TEXT_ACTION:
              try {
                final ArrayList<Object> argumentList = (ArrayList<Object>) args;
                int id = (int) (argumentList.get(0));
                String text = (String) (argumentList.get(1));
                boolean readOnly = (boolean) (argumentList.get(2));
                processTextMethodHandler.processTextAction(id, text, readOnly, result);
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

  public ProcessTextChannel(
      @NonNull DartExecutor dartExecutor, @NonNull PackageManager packageManager) {
    this.packageManager = packageManager;
    channel = new MethodChannel(dartExecutor, CHANNEL_NAME, StandardMethodCodec.INSTANCE);
    channel.setMethodCallHandler(parsingMethodHandler);
  }

  /**
   * Sets the {@link ProcessTextMethodHandler} which receives all requests to the text processing
   * feature sent through this channel.
   */
  public void setMethodHandler(@Nullable ProcessTextMethodHandler processTextMethodHandler) {
    this.processTextMethodHandler = processTextMethodHandler;
  }

  public interface ProcessTextMethodHandler {
    /**
     * Requests the list of text actions. Each text action has a unique id and a localized label.
     */
    Map<Integer, String> queryTextActions();

    /**
     * Requests to run a text action on a given input text.
     *
     * <p>TODO(bleroux): add documentation for parameters.
     */
    void processTextAction(
        @NonNull int id,
        @NonNull String input,
        @NonNull boolean readOnly,
        @NonNull MethodChannel.Result result);
  }
}
