// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.shim;

import androidx.annotation.NonNull;

import io.flutter.Log;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.activity.ActivityAware;
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding;
import io.flutter.plugin.common.PluginRegistry;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * TODO what is this doing, also clean up unused imports
 *
 * <p>Instances of {@code ShimRegistrar}s are vended internally by a {@link ShimPluginRegistry}.
 */
class ShimRegistrar implements FlutterPlugin, ActivityAware {
  private static final String TAG = "ShimRegistrar";

  private final Map<String, Object> globalRegistrarMap;
  private final String pluginId;
  private final Set<PluginRegistry.RequestPermissionsResultListener>
      requestPermissionsResultListeners = new HashSet<>();
  private final Set<PluginRegistry.ActivityResultListener> activityResultListeners =
      new HashSet<>();
  private final Set<PluginRegistry.NewIntentListener> newIntentListeners = new HashSet<>();
  private final Set<PluginRegistry.UserLeaveHintListener> userLeaveHintListeners = new HashSet<>();
  private final Set<PluginRegistry.WindowFocusChangedListener> WindowFocusChangedListeners =
      new HashSet<>();
  private FlutterPlugin.FlutterPluginBinding pluginBinding;
  private ActivityPluginBinding activityPluginBinding;

  public ShimRegistrar(@NonNull String pluginId, @NonNull Map<String, Object> globalRegistrarMap) {
    this.pluginId = pluginId;
    this.globalRegistrarMap = globalRegistrarMap;
  }

  @Override
  public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
    Log.v(TAG, "Attached to FlutterEngine.");
    pluginBinding = binding;
  }

  @Override
  public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
    Log.v(TAG, "Detached from FlutterEngine.");

    pluginBinding = null;
    activityPluginBinding = null;
  }

  @Override
  public void onAttachedToActivity(@NonNull ActivityPluginBinding binding) {
    Log.v(TAG, "Attached to an Activity.");
    activityPluginBinding = binding;
    addExistingListenersToActivityPluginBinding();
  }

  @Override
  public void onDetachedFromActivityForConfigChanges() {
    Log.v(TAG, "Detached from an Activity for config changes.");
    activityPluginBinding = null;
  }

  @Override
  public void onReattachedToActivityForConfigChanges(@NonNull ActivityPluginBinding binding) {
    Log.v(TAG, "Reconnected to an Activity after config changes.");
    activityPluginBinding = binding;
    addExistingListenersToActivityPluginBinding();
  }

  @Override
  public void onDetachedFromActivity() {
    Log.v(TAG, "Detached from an Activity.");
    activityPluginBinding = null;
  }

  private void addExistingListenersToActivityPluginBinding() {
    for (PluginRegistry.RequestPermissionsResultListener listener :
        requestPermissionsResultListeners) {
      activityPluginBinding.addRequestPermissionsResultListener(listener);
    }
    for (PluginRegistry.ActivityResultListener listener : activityResultListeners) {
      activityPluginBinding.addActivityResultListener(listener);
    }
    for (PluginRegistry.NewIntentListener listener : newIntentListeners) {
      activityPluginBinding.addOnNewIntentListener(listener);
    }
    for (PluginRegistry.UserLeaveHintListener listener : userLeaveHintListeners) {
      activityPluginBinding.addOnUserLeaveHintListener(listener);
    }
    for (PluginRegistry.WindowFocusChangedListener listener : WindowFocusChangedListeners) {
      activityPluginBinding.addOnWindowFocusChangedListener(listener);
    }
  }
}
