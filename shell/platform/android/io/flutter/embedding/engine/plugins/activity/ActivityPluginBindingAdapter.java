package io.flutter.embedding.engine.plugins.activity;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import io.flutter.embedding.android.HostComponent;
import io.flutter.embedding.engine.plugins.host.HostComponentPluginBinding;
import io.flutter.plugin.common.PluginRegistry;

public class ActivityPluginBindingAdapter implements ActivityPluginBinding {

  private final HostComponentPluginBinding hostComponentPluginBinding;

  public ActivityPluginBindingAdapter(HostComponentPluginBinding hostComponentPluginBinding) {
    this.hostComponentPluginBinding = hostComponentPluginBinding;
  }

  @NonNull
  @Override
  public Context getContext() {
    return hostComponentPluginBinding.getContext();
  }

  @NonNull
  @Override
  public Activity getActivity() {
    return hostComponentPluginBinding.getActivity();
  }

  @NonNull
  @Override
  public HostComponent getHostComponent() {
    return hostComponentPluginBinding.getHostComponent();
  }

  @NonNull
  @Override
  public Object getLifecycle() {
    return hostComponentPluginBinding.getLifecycle();
  }

  @Override
  public void addRequestPermissionsResultListener(
      @NonNull PluginRegistry.RequestPermissionsResultListener listener) {
    hostComponentPluginBinding.addRequestPermissionsResultListener(listener);
  }

  @Override
  public void removeRequestPermissionsResultListener(
      @NonNull PluginRegistry.RequestPermissionsResultListener listener) {
    hostComponentPluginBinding.removeRequestPermissionsResultListener(listener);
  }

  @Override
  public void addActivityResultListener(@NonNull PluginRegistry.ActivityResultListener listener) {
    hostComponentPluginBinding.addActivityResultListener(listener);
  }

  @Override
  public void removeActivityResultListener(
      @NonNull PluginRegistry.ActivityResultListener listener) {
    hostComponentPluginBinding.removeActivityResultListener(listener);
  }

  @Override
  public void addOnNewIntentListener(@NonNull PluginRegistry.NewIntentListener listener) {
    hostComponentPluginBinding.addOnNewIntentListener(listener);
  }

  @Override
  public void removeOnNewIntentListener(@NonNull PluginRegistry.NewIntentListener listener) {
    hostComponentPluginBinding.removeOnNewIntentListener(listener);
  }

  @Override
  public void addOnUserLeaveHintListener(@NonNull PluginRegistry.UserLeaveHintListener listener) {
    hostComponentPluginBinding.addOnUserLeaveHintListener(listener);
  }

  @Override
  public void removeOnUserLeaveHintListener(
      @NonNull PluginRegistry.UserLeaveHintListener listener) {
    hostComponentPluginBinding.removeOnUserLeaveHintListener(listener);
  }

  @Override
  public void addOnSaveStateListener(
      @NonNull HostComponentPluginBinding.OnSaveInstanceStateListener listener) {
    hostComponentPluginBinding.addOnSaveStateListener(listener);
  }

  @Override
  public void removeOnSaveStateListener(
      @NonNull HostComponentPluginBinding.OnSaveInstanceStateListener listener) {
    hostComponentPluginBinding.removeOnSaveStateListener(listener);
  }
}
