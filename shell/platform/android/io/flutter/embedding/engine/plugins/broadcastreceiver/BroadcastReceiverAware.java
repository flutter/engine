// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.broadcastreceiver;

import android.support.annotation.NonNull;

/**
 * {@link FlutterPlugin} that wants to know when it is running within a {@link BroadcastReceiver}.
 */
public interface BroadcastReceiverAware {
  /**
   * This {@code BroadcastReceiverAware} {@link FlutterPlugin} is now associated with a
   * {@link BroadcastReceiver}.
   */
  void onAttachedToBroadcastReceiver(@NonNull BroadcastReceiverPluginBinding binding);

  /**
   * This plugin has been detached from a {@link BroadcastReceiver}.
   */
  void onDetachedFromBroadcastReceiver();
}
