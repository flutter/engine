// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.view.Window;
import android.view.WindowInsets;
import androidx.annotation.RequiresApi;
import androidx.annotation.VisibleForTesting;
import io.flutter.Build;
import io.flutter.util.ViewUtils;
import java.util.Collections;
import java.util.List;

/** A delegate class that performs the task of retrieving the bounding rect values. */
public class FlutterViewDelegate {
  /**
   * Return the WindowInsets object for the provided Context. Returns null if there is no associated
   * activity, the window of the associated activity is null, or the root window insets of the
   * activity's window is null.
   */
  @RequiresApi(api = Build.API_LEVELS.API_23)
  @VisibleForTesting
  public WindowInsets getWindowInsets(Context context) {
    Activity activity = ViewUtils.getActivity(context);
    if (activity == null) {
      return null;
    }
    Window window = activity.getWindow();
    if (window == null) {
      return null;
    }
    return window.getDecorView().getRootWindowInsets();
  }

  @RequiresApi(api = Build.API_LEVELS.API_35)
  @VisibleForTesting
  public List<Rect> getCaptionBarInsets(Context context) {
    WindowInsets insets = getWindowInsets(context);
    if (insets == null) {
      return Collections.emptyList();
    }
    return insets.getBoundingRects(WindowInsets.Type.captionBar());
  }
}