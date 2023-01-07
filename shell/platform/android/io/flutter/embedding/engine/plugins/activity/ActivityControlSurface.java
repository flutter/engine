// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.activity;

import android.app.Activity;
import androidx.annotation.NonNull;
import androidx.lifecycle.Lifecycle;
import io.flutter.embedding.android.ExclusiveAppComponent;
import io.flutter.embedding.engine.plugins.host.HostComponentControlSurface;

/**
 * Control surface through which an {@link android.app.Activity} attaches to a {@link
 * io.flutter.embedding.engine.FlutterEngine}.
 *
 * <p>An {@link android.app.Activity} that contains a {@link
 * io.flutter.embedding.android.FlutterView} and associated {@link
 * io.flutter.embedding.engine.FlutterEngine} should coordinate itself with the {@link
 * io.flutter.embedding.engine.FlutterEngine}'s {@code ActivityControlSurface}.
 *
 * <ol>
 *   <li>Once an {@link android.app.Activity} is created, and its associated {@link
 *       io.flutter.embedding.engine.FlutterEngine} is executing Dart code, the {@link
 *       android.app.Activity} should invoke {@link #attachToActivity( ExclusiveAppComponent,
 *       Lifecycle)}. At this point the {@link io.flutter.embedding.engine.FlutterEngine} is
 *       considered "attached" to the {@link android.app.Activity} and all {@link ActivityAware}
 *       plugins are given access to the {@link android.app.Activity}.
 *   <li>Just before an attached {@link android.app.Activity} is destroyed for configuration change
 *       purposes, that {@link android.app.Activity} should invoke {@link
 *       #detachFromActivityForConfigChanges()}, giving each {@link ActivityAware} plugin an
 *       opportunity to clean up its references before the {@link android.app.Activity is
 *       destroyed}.
 *   <li>When an {@link android.app.Activity} is destroyed for non-configuration-change purposes, or
 *       when the {@link android.app.Activity} is no longer interested in displaying a {@link
 *       io.flutter.embedding.engine.FlutterEngine}'s content, the {@link android.app.Activity}
 *       should invoke {@link #detachFromActivity()}.
 *   <li>When a {@link android.app.Activity} is being attached while an existing {@link
 *       ExclusiveAppComponent} is already attached, the existing {@link ExclusiveAppComponent} is
 *       given a chance to detach first via {@link ExclusiveAppComponent#detachFromFlutterEngine()}
 *       before the new activity attaches.
 * </ol>
 *
 * The attached {@link android.app.Activity} should also forward all {@link android.app.Activity}
 * calls that this {@code ActivityControlSurface} supports, e.g., {@link
 * #onRequestPermissionsResult(int, String[], int[])}. These forwarded calls are made available to
 * all {@link ActivityAware} plugins that are added to the attached {@link
 * io.flutter.embedding.engine.FlutterEngine}.
 */
public interface ActivityControlSurface extends HostComponentControlSurface {
  /**
   * Call this method from the {@link ExclusiveAppComponent} that is displaying the visual content
   * of the {@link io.flutter.embedding.engine.FlutterEngine} that is associated with this {@code
   * ActivityControlSurface}.
   *
   * <p>Once an {@link ExclusiveAppComponent} is created, and its associated {@link
   * io.flutter.embedding.engine.FlutterEngine} is executing Dart code, the {@link
   * ExclusiveAppComponent} should invoke this method. At that point the {@link
   * io.flutter.embedding.engine.FlutterEngine} is considered "attached" to the {@link
   * ExclusiveAppComponent} and all {@link ActivityAware} plugins are given access to the {@link
   * ExclusiveAppComponent}'s {@link android.app.Activity}.
   */
  void attachToActivity(
      @NonNull ExclusiveAppComponent<Activity> exclusiveActivity, @NonNull Lifecycle lifecycle);

  /**
   * Call this method from the {@link android.app.Activity} that is attached to this {@code
   * ActivityControlSurfaces}'s {@link io.flutter.embedding.engine.FlutterEngine} when the {@link
   * android.app.Activity} is about to be destroyed due to configuration changes.
   *
   * <p>This method gives each {@link ActivityAware} plugin an opportunity to clean up its
   * references before the {@link android.app.Activity is destroyed}.
   */
  void detachFromActivityForConfigChanges();

  /**
   * Call this method from the {@link android.app.Activity} that is attached to this {@code
   * ActivityControlSurfaces}'s {@link io.flutter.embedding.engine.FlutterEngine} when the {@link
   * android.app.Activity} is about to be destroyed for non-configuration-change reasons.
   *
   * <p>This method gives each {@link ActivityAware} plugin an opportunity to clean up its
   * references before the {@link android.app.Activity is destroyed}.
   */
  void detachFromActivity();
}
