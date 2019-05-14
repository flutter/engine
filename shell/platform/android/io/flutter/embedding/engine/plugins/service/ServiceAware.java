// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.service;

import android.support.annotation.NonNull;

/**
 * {@link FlutterPlugin} that wants to know when it is running within a {@link Service}.
 */
public interface ServiceAware {
  /**
   * This {@code ServiceAware} {@link FlutterPlugin} is now associated with a {@link Service}.
   */
  void onAttachedToService(@NonNull ServicePluginBinding binding);

  /**
   * This plugin has been detached from a {@link Service}.
   */
  void onDetachedFromService();

  interface OnModeChangeListener {
    /**
     * The associated {@link Service} has changed from a background {@link Service} to a foreground
     * {@link Service}.
     */
    void onMoveToForeground();

    /**
     * The associated {@link Service} has changed from a foreground {@link Service} to a background
     * {@link Service}.
     */
    void onMoveToBackground();
  }
}
