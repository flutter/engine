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
    // final Window flutterViewWindow = flutterView.getWindow();
    WindowId flutterWindowId = flutterView.getWindowId();
    IBinder windowId = flutterView.getWindowToken();

    Log.e("CAMILLE", Integer.toString(i) + ": flutter view ID: " + Integer.toString(flutterViewId));
    Log.e("CAMILLE", Integer.toString(i) + ": flutter view: " + flutterView.toString());
    Log.e("CAMILLE", Integer.toString(i) + ": flutter view is visible: " + Integer.toString(flutterView.getVisibility()));
    Log.e("CAMILLE", Integer.toString(i) + ": flutter window ID : " + flutterWindowId);
    Log.e("CAMILLE", Integer.toString(i) + ": flutter view window token: " + windowId);


    if (flutterView == null) {
      result.error("error", "Requested Flutter View to set content sensitivty of not found.", null);
    }
    
    int currentSensitivity = flutterView.getContentSensitivity();
    Log.e("CAMILLE", Integer.toString(i) + ": current content sensitivity is: " + Integer.toString(currentSensitivity));


    flutterView.setContentSensitivity(contentSensitivity);
    int newSensitivity = flutterView.getContentSensitivity();

    Log.e("CAMILLE", Integer.toString(i) + ": set content sensitivity to: " + Integer.toString(contentSensitivity));
    Log.e("CAMILLE", Integer.toString(i) + ": acutal new content sensitivity is: " + Integer.toString(newSensitivity));
    result.success(null);
    i++;
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
