// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.view;

import android.app.Activity;
import android.view.View;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.systemchannels.SensitiveContentChannel;
import io.flutter.plugin.common.MethodChannel;

import io.flutter.Log;
import android.os.IBinder;
import android.view.Window;
import android.view.WindowId;

/**
 * {@link SensitiveContentPlugin} is the implementation of all functionality needed to set content
 * sensitive on a native Flutter Android {@code View}.
 *
 * <p>The plugin handles requests for setting content sensitivity sent by the {@link
 * io.flutter.embedding.engine.systemchannels.SensitiveContentChannel} via making a call to the
 * relevant {@code View}.
 */
public class SensitiveContentPlugin
    implements SensitiveContentChannel.SensitiveContentMethodHandler {

  private final Activity mflutterActivity;
  private final SensitiveContentChannel mSensitiveContentChannel;

  public SensitiveContentPlugin(
      @NonNull Activity flutterActivity, @NonNull SensitiveContentChannel sensitiveContentChannel) {
    mflutterActivity = flutterActivity;
    mSensitiveContentChannel = sensitiveContentChannel;

    mSensitiveContentChannel.setSensitiveContentMethodHandler(this);
  }

  int i = 0;
  /**
   * Sets content sensitivity level of the Android {@code View} with the specified {@code
   * flutterViewId} to the level specified by {@contentSensitivity}.
   */
  @Override
  public void setContentSensitivity(
      @NonNull int flutterViewId,
      @NonNull int contentSensitivity,
      @NonNull MethodChannel.Result result) {
    final View flutterView = mflutterActivity.findViewById(flutterViewId);
    if (flutterView == null) {
      result.error("error", "Requested Flutter View to set content sensitivty of not found.", null);
    }

    final int currentContentSensitivity = flutterView.getContentSensitivity();
    flutterView.setContentSensitivity(contentSensitivity);

    final boolean shouldInvalidateView = currentContentSensitivity == View.CONTENT_SENSITIVITY_SENSITIVE && contentSensitivity != View.CONTENT_SENSITIVITY_SENSITIVE;
    if (shouldInvalidateView) {
      flutterView.invalidate();
    }

    result.success(null);
  }

  /**
   * Releases all resources held by this {@code SensitiveContentPlugin}.
   *
   * <p>Do not invoke any methods on a {@code SensitiveContentPlugin} after invoking this method.
   */
  public void destroy() {
    this.mSensitiveContentChannel.setSensitiveContentMethodHandler(null);
  }
}
