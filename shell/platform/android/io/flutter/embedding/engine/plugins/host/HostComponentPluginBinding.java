// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.host;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.plugin.common.PluginRegistry;

/**
 * Binding that gives {@link HostComponentAware} plugins access to an associated {@link
 * HostComponent} and the {@link HostComponent}'s lifecycle methods.
 *
 * <p>To obtain an instance of an {@code HostComponentPluginBinding} in a Flutter plugin, implement
 * the {@link HostComponentAware} interface. A binding is provided in {@link
 * HostComponentAware#onAttachedToHostComponent(HostComponentPluginBinding)} and {@link
 * HostComponentAware#onReattachedToHostComponentForConfigChanges(HostComponentPluginBinding)}.
 */
public interface HostComponentPluginBinding {

  @NonNull
  Context getContext();

  /**
   * Returns the {@link HostComponent} that is currently attached to the {@link
   * io.flutter.embedding.engine.FlutterEngine} that owns this {@code ActivityPluginBinding}.
   */
  Activity getActivity();

  @NonNull
  HostComponent getHostComponent();

  /**
   * Returns the {@code Lifecycle} associated with the attached {@code HostComponent}.
   *
   * <p>Use the flutter_plugin_android_lifecycle plugin to turn the returned {@code Object} into a
   * {@code Lifecycle} object. See
   * (https://github.com/flutter/plugins/tree/master/packages/flutter_plugin_android_lifecycle).
   * Flutter plugins that rely on {@code Lifecycle} are forced to use the
   * flutter_plugin_android_lifecycle plugin so that the version of the Android Lifecycle library is
   * exposed to pub, which allows Flutter to manage different versions library over time.
   */
  @NonNull
  Object getLifecycle();

  /**
   * Adds a listener that is invoked whenever the associated {@link HostComponent}'s {@code
   * onRequestPermissionsResult(...)} method is invoked.
   */
  void addRequestPermissionsResultListener(
      @NonNull PluginRegistry.RequestPermissionsResultListener listener);

  /**
   * Removes a listener that was added in {@link
   * #addRequestPermissionsResultListener(PluginRegistry.RequestPermissionsResultListener)}.
   */
  void removeRequestPermissionsResultListener(
      @NonNull PluginRegistry.RequestPermissionsResultListener listener);

  /**
   * Adds a listener that is invoked whenever the associated {@link HostComponent}'s {@code
   * onActivityResult(...)} method is invoked.
   */
  void addActivityResultListener(@NonNull PluginRegistry.ActivityResultListener listener);

  /**
   * Removes a listener that was added in {@link
   * #addActivityResultListener(PluginRegistry.ActivityResultListener)}.
   */
  void removeActivityResultListener(@NonNull PluginRegistry.ActivityResultListener listener);

  /**
   * Adds a listener that is invoked whenever the associated {@link HostComponent}'s {@code
   * onNewIntent(...)} method is invoked.
   */
  void addOnNewIntentListener(@NonNull PluginRegistry.NewIntentListener listener);

  /**
   * Removes a listener that was added in {@link
   * #addOnNewIntentListener(PluginRegistry.NewIntentListener)}.
   */
  void removeOnNewIntentListener(@NonNull PluginRegistry.NewIntentListener listener);

  /**
   * Adds a listener that is invoked whenever the associated {@link HostComponent}'s {@code
   * onUserLeaveHint()} method is invoked.
   */
  void addOnUserLeaveHintListener(@NonNull PluginRegistry.UserLeaveHintListener listener);

  /**
   * Removes a listener that was added in {@link
   * #addOnUserLeaveHintListener(PluginRegistry.UserLeaveHintListener)}.
   */
  void removeOnUserLeaveHintListener(@NonNull PluginRegistry.UserLeaveHintListener listener);

  /**
   * Adds a listener that is invoked when the associated {@code Activity} or {@code Fragment} saves
   * and restores instance state.
   */
  void addOnSaveStateListener(@NonNull OnSaveInstanceStateListener listener);

  /**
   * Removes a listener that was added in {@link
   * #addOnSaveStateListener(OnSaveInstanceStateListener)}.
   */
  void removeOnSaveStateListener(@NonNull OnSaveInstanceStateListener listener);

  interface OnSaveInstanceStateListener {
    /**
     * Invoked when the associated {@code Activity} or {@code Fragment} executes {@link
     * Activity#onSaveInstanceState(Bundle)}.
     */
    void onSaveInstanceState(@NonNull Bundle bundle);

    /**
     * Invoked when the associated {@code Activity} executes {@link
     * android.app.Activity#onCreate(Bundle)} or associated {@code Fragment} executes {@code
     * Fragment#onCreate(Bundle)}.
     */
    void onRestoreInstanceState(@Nullable Bundle bundle);
  }
}
