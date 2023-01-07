// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.host;

import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.Lifecycle;
import io.flutter.embedding.android.ExclusiveAppComponent;

/**
 * Control surface through which an {@link HostComponent} attaches to a {@link
 * io.flutter.embedding.engine.FlutterEngine}.
 *
 * <p>An {@link HostComponent} that contains a {@link io.flutter.embedding.android.FlutterView} and
 * associated {@link io.flutter.embedding.engine.FlutterEngine} should coordinate itself with the
 * {@link io.flutter.embedding.engine.FlutterEngine}'s {@code HostComponentControlSurface}.
 *
 * <ol>
 *   <li>Once an {@link HostComponent} is created, and its associated {@link
 *       io.flutter.embedding.engine.FlutterEngine} is executing Dart code, the {@link
 *       HostComponent} should invoke {@link #attachToHostComponent( ExclusiveAppComponent,
 *       Lifecycle)}. At this point the {@link io.flutter.embedding.engine.FlutterEngine} is
 *       considered "attached" to the {@link HostComponent} and all {@link HostComponentAware}
 *       plugins are given access to the {@link HostComponent}.
 *   <li>Just before an attached {@link HostComponent} is destroyed for configuration change
 *       purposes, that {@link HostComponent} should invoke {@link
 *       #detachFromHostComponentForConfigChanges()}, giving each {@link HostComponentAware} plugin
 *       an opportunity to clean up its references before the {@link HostComponent is destroyed}.
 *   <li>When an {@link HostComponent} is destroyed for non-configuration-change purposes, or when
 *       the {@link HostComponent} is no longer interested in displaying a {@link
 *       io.flutter.embedding.engine.FlutterEngine}'s content, the {@link HostComponent} should
 *       invoke {@link #detachFromHostComponent()}.
 *   <li>When a {@link HostComponent} is being attached while an existing {@link
 *       ExclusiveAppComponent} is already attached, the existing {@link ExclusiveAppComponent} is
 *       given a chance to detach first via {@link ExclusiveAppComponent#detachFromFlutterEngine()}
 *       before the new host component attaches.
 * </ol>
 *
 * The attached {@link HostComponent} should also forward all {@link HostComponent} calls that this
 * {@code HostComponentControlSurface} supports, e.g., {@link #onRequestPermissionsResult(int,
 * String[], int[])}. These forwarded calls are made available to all {@link HostComponentAware}
 * plugins that are added to the attached {@link io.flutter.embedding.engine.FlutterEngine}.
 */
public interface HostComponentControlSurface {

  /**
   * Call this method from the {@link ExclusiveAppComponent} that is displaying the visual content
   * of the {@link io.flutter.embedding.engine.FlutterEngine} that is associated with this {@code
   * HostComponentControlSurface}.
   *
   * <p>Once an {@link ExclusiveAppComponent} is created, and its associated {@link
   * io.flutter.embedding.engine.FlutterEngine} is executing Dart code, the {@link
   * ExclusiveAppComponent} should invoke this method. At that point the {@link
   * io.flutter.embedding.engine.FlutterEngine} is considered "attached" to the {@link
   * ExclusiveAppComponent} and all {@link HostComponentAware} plugins are given access to the
   * {@link ExclusiveAppComponent}'s {@link HostComponent}.
   */
  void attachToHostComponent(
      @NonNull ExclusiveAppComponent<HostComponent> exclusiveActivity,
      @NonNull Lifecycle lifecycle);

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} when the
   * {@link HostComponent} is about to be destroyed due to configuration changes.
   *
   * <p>This method gives each {@link HostComponentAware} plugin an opportunity to clean up its
   * references before the {@link HostComponent is destroyed}.
   */
  void detachFromHostComponentForConfigChanges();

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} when the
   * {@link HostComponent} is about to be destroyed for non-configuration-change reasons.
   *
   * <p>This method gives each {@link HostComponentAware} plugin an opportunity to clean up its
   * references before the {@link HostComponent is destroyed}.
   */
  void detachFromHostComponent();

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} and the
   * associated method in the {@link HostComponent} or Fragment is invoked.
   *
   * <p>Returns true if one or more plugins utilized this permission result.
   */
  boolean onRequestPermissionsResult(
      int requestCode, @NonNull String[] permissions, @NonNull int[] grantResult);

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} and the
   * associated method in the {@link HostComponent} is invoked.
   *
   * <p>Returns true if one or more plugins utilized this {@link HostComponent} result.
   */
  boolean onActivityResult(int requestCode, int resultCode, @Nullable Intent data);

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} and the
   * associated method in the {@link HostComponent} is invoked.
   */
  void onNewIntent(@NonNull Intent intent);

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * HostComponentControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} and the
   * associated method in the {@link HostComponent} is invoked.
   */
  void onUserLeaveHint();

  /**
   * Call this method from the {@link HostComponent} or {@code Fragment} that is attached to this
   * {@code ActivityControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} when the
   * associated method is invoked in the {@link HostComponent}.
   */
  void onSaveInstanceState(@NonNull Bundle bundle);

  /**
   * Call this method from the {@link HostComponent} that is attached to this {@code
   * ActivityControlSurface}'s {@link io.flutter.embedding.engine.FlutterEngine} when {@link
   * android.app.Activity#onCreate(Bundle)} or {@code Fragment#onCreate(Bundle)} is invoked in the
   * {@link HostComponent}.
   */
  void onRestoreInstanceState(@Nullable Bundle bundle);
}
