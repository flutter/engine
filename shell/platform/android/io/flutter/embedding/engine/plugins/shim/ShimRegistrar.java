// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.shim;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import io.flutter.FlutterInjector;
import io.flutter.Log;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.host.HostComponentAware;
import io.flutter.embedding.engine.plugins.host.HostComponentPluginBinding;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.PluginRegistry;
import io.flutter.plugin.platform.PlatformViewRegistry;
import io.flutter.view.FlutterView;
import io.flutter.view.TextureRegistry;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * A {@link PluginRegistry.Registrar} that is shimmed let old plugins use the new Android embedding
 * and plugin API behind the scenes.
 *
 * <p>Instances of {@code ShimRegistrar}s are vended internally by a {@link ShimPluginRegistry}.
 */
class ShimRegistrar implements PluginRegistry.Registrar, FlutterPlugin, HostComponentAware {
  private static final String TAG = "ShimRegistrar";

  private final Map<String, Object> globalRegistrarMap;
  private final String pluginId;
  private final Set<PluginRegistry.ViewDestroyListener> viewDestroyListeners = new HashSet<>();
  private final Set<PluginRegistry.RequestPermissionsResultListener>
      requestPermissionsResultListeners = new HashSet<>();
  private final Set<PluginRegistry.ActivityResultListener> activityResultListeners =
      new HashSet<>();
  private final Set<PluginRegistry.NewIntentListener> newIntentListeners = new HashSet<>();
  private final Set<PluginRegistry.UserLeaveHintListener> userLeaveHintListeners = new HashSet<>();
  private FlutterPlugin.FlutterPluginBinding pluginBinding;
  private HostComponentPluginBinding hostComponentPluginBinding;

  public ShimRegistrar(@NonNull String pluginId, @NonNull Map<String, Object> globalRegistrarMap) {
    this.pluginId = pluginId;
    this.globalRegistrarMap = globalRegistrarMap;
  }

  @Override
  public Activity activity() {
    return hostComponentPluginBinding != null ? hostComponentPluginBinding.getActivity() : null;
  }

  @Override
  public Context context() {
    return pluginBinding != null ? pluginBinding.getApplicationContext() : null;
  }

  @Override
  public Context activeContext() {
    return hostComponentPluginBinding == null ? context() : hostComponentPluginBinding.getContext();
  }

  @Override
  public BinaryMessenger messenger() {
    return pluginBinding != null ? pluginBinding.getBinaryMessenger() : null;
  }

  @Override
  public TextureRegistry textures() {
    return pluginBinding != null ? pluginBinding.getTextureRegistry() : null;
  }

  @Override
  public PlatformViewRegistry platformViewRegistry() {
    return pluginBinding != null ? pluginBinding.getPlatformViewRegistry() : null;
  }

  @Override
  public FlutterView view() {
    throw new UnsupportedOperationException(
        "The new embedding does not support the old FlutterView.");
  }

  @Override
  public String lookupKeyForAsset(String asset) {
    return FlutterInjector.instance().flutterLoader().getLookupKeyForAsset(asset);
  }

  @Override
  public String lookupKeyForAsset(String asset, String packageName) {
    return FlutterInjector.instance().flutterLoader().getLookupKeyForAsset(asset, packageName);
  }

  @Override
  public PluginRegistry.Registrar publish(Object value) {
    globalRegistrarMap.put(pluginId, value);
    return this;
  }

  @Override
  public PluginRegistry.Registrar addRequestPermissionsResultListener(
      PluginRegistry.RequestPermissionsResultListener listener) {
    requestPermissionsResultListeners.add(listener);

    if (hostComponentPluginBinding != null) {
      hostComponentPluginBinding.addRequestPermissionsResultListener(listener);
    }

    return this;
  }

  @Override
  public PluginRegistry.Registrar addActivityResultListener(
      PluginRegistry.ActivityResultListener listener) {
    activityResultListeners.add(listener);

    if (hostComponentPluginBinding != null) {
      hostComponentPluginBinding.addActivityResultListener(listener);
    }

    return this;
  }

  @Override
  public PluginRegistry.Registrar addNewIntentListener(PluginRegistry.NewIntentListener listener) {
    newIntentListeners.add(listener);

    if (hostComponentPluginBinding != null) {
      hostComponentPluginBinding.addOnNewIntentListener(listener);
    }

    return this;
  }

  @Override
  public PluginRegistry.Registrar addUserLeaveHintListener(
      PluginRegistry.UserLeaveHintListener listener) {
    userLeaveHintListeners.add(listener);

    if (hostComponentPluginBinding != null) {
      hostComponentPluginBinding.addOnUserLeaveHintListener(listener);
    }

    return this;
  }

  @Override
  @NonNull
  public PluginRegistry.Registrar addViewDestroyListener(
      @NonNull PluginRegistry.ViewDestroyListener listener) {
    viewDestroyListeners.add(listener);
    return this;
  }

  @Override
  public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
    Log.v(TAG, "Attached to FlutterEngine.");
    pluginBinding = binding;
  }

  @Override
  public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
    Log.v(TAG, "Detached from FlutterEngine.");
    for (PluginRegistry.ViewDestroyListener listener : viewDestroyListeners) {
      // The following invocation might produce unexpected behavior in old plugins because
      // we have no FlutterNativeView to pass to onViewDestroy(). This is a limitation of this shim.
      listener.onViewDestroy(null);
    }

    pluginBinding = null;
    hostComponentPluginBinding = null;
  }

  @Override
  public void onAttachedToHostComponent(@NonNull HostComponentPluginBinding binding) {
    Log.v(TAG, "Attached to an Activity.");
    hostComponentPluginBinding = binding;
    addExistingListenersToHostComponentPluginBinding();
  }

  @Override
  public void onDetachedFromHostComponentForConfigChanges() {
    Log.v(TAG, "Detached from an Activity for config changes.");
    hostComponentPluginBinding = null;
  }

  @Override
  public void onReattachedToHostComponentForConfigChanges(
      @NonNull HostComponentPluginBinding binding) {
    Log.v(TAG, "Reconnected to an Activity after config changes.");
    hostComponentPluginBinding = binding;
    addExistingListenersToHostComponentPluginBinding();
  }

  @Override
  public void onDetachedFromHostComponent() {
    Log.v(TAG, "Detached from an Activity.");
    hostComponentPluginBinding = null;
  }

  private void addExistingListenersToHostComponentPluginBinding() {
    for (PluginRegistry.RequestPermissionsResultListener listener :
        requestPermissionsResultListeners) {
      hostComponentPluginBinding.addRequestPermissionsResultListener(listener);
    }
    for (PluginRegistry.ActivityResultListener listener : activityResultListeners) {
      hostComponentPluginBinding.addActivityResultListener(listener);
    }
    for (PluginRegistry.NewIntentListener listener : newIntentListeners) {
      hostComponentPluginBinding.addOnNewIntentListener(listener);
    }
    for (PluginRegistry.UserLeaveHintListener listener : userLeaveHintListeners) {
      hostComponentPluginBinding.addOnUserLeaveHintListener(listener);
    }
  }
}
