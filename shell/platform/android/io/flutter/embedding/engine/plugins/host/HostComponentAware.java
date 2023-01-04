// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.host;

import androidx.annotation.NonNull;

/**
 * {@link io.flutter.embedding.engine.plugins.FlutterPlugin} that is interested in {@link
 * android.app.Activity} lifecycle events related to a {@link
 * io.flutter.embedding.engine.FlutterEngine} running within the given {@link android.app.Activity}.
 */
public interface HostComponentAware {
  /**
   * This {@code ActivityAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin} is now
   * associated with an {@link android.app.Activity}.
   *
   * <p>This method can be invoked in 1 of 2 situations:
   *
   * <ul>
   *   <li>This {@code ActivityAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin} was
   *       just added to a {@link io.flutter.embedding.engine.FlutterEngine} that was already
   *       connected to a running {@link android.app.Activity}.
   *   <li>This {@code ActivityAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin} was
   *       already added to a {@link io.flutter.embedding.engine.FlutterEngine} and that {@link
   *       io.flutter.embedding.engine.FlutterEngine} was just connected to an {@link
   *       android.app.Activity}.
   * </ul>
   *
   * <p>The given {@link HostComponentPluginBinding} contains {@link android.app.Activity}-related
   * references that an {@code ActivityAware} {@link
   * io.flutter.embedding.engine.plugins.FlutterPlugin} may require, such as a reference to the
   * actual {@link android.app.Activity} in question. The {@link HostComponentPluginBinding} may be
   * referenced until either {@link #onDetachedFromHostComponentForConfigChanges()} or {@link
   * #onDetachedFromHostComponent()} is invoked. At the conclusion of either of those methods, the
   * binding is no longer valid. Clear any references to the binding or its resources, and do not
   * invoke any further methods on the binding or its resources.
   */
  void onAttachedToHostComponent(@NonNull HostComponentPluginBinding binding);

  /**
   * The {@link android.app.Activity} that was attached and made available in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} has been detached from this {@code
   * ActivityAware}'s {@link io.flutter.embedding.engine.FlutterEngine} for the purpose of
   * processing a configuration change.
   *
   * <p>By the end of this method, the {@link android.app.Activity} that was made available in
   * {@link #onAttachedToHostComponent(HostComponentPluginBinding)} is no longer valid. Any
   * references to the associated {@link android.app.Activity} or {@link HostComponentPluginBinding}
   * should be cleared.
   *
   * <p>This method should be quickly followed by {@link
   * #onReattachedToHostComponentForConfigChanges(HostComponentPluginBinding)}, which signifies that
   * a new {@link android.app.Activity} has been created with the new configuration options. That
   * method provides a new {@link HostComponentPluginBinding}, which references the newly created
   * and associated {@link android.app.Activity}.
   *
   * <p>Any {@code Lifecycle} listeners that were registered in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} should be deregistered here to avoid a
   * possible memory leak and other side effects.
   */
  void onDetachedFromHostComponentForConfigChanges();

  /**
   * This plugin and its {@link io.flutter.embedding.engine.FlutterEngine} have been re-attached to
   * an {@link android.app.Activity} after the {@link android.app.Activity} was recreated to handle
   * configuration changes.
   *
   * <p>{@code binding} includes a reference to the new instance of the {@link
   * android.app.Activity}. {@code binding} and its references may be cached and used from now until
   * either {@link #onDetachedFromHostComponentForConfigChanges()} or {@link
   * #onDetachedFromHostComponent()} is invoked. At the conclusion of either of those methods, the
   * binding is no longer valid. Clear any references to the binding or its resources, and do not
   * invoke any further methods on the binding or its resources.
   */
  void onReattachedToHostComponentForConfigChanges(@NonNull HostComponentPluginBinding binding);

  /**
   * This plugin has been detached from an {@link android.app.Activity}.
   *
   * <p>Detachment can occur for a number of reasons.
   *
   * <ul>
   *   <li>The app is no longer visible and the {@link android.app.Activity} instance has been
   *       destroyed.
   *   <li>The {@link io.flutter.embedding.engine.FlutterEngine} that this plugin is connected to
   *       has been detached from its {@link io.flutter.embedding.android.FlutterView}.
   *   <li>This {@code ActivityAware} plugin has been removed from its {@link
   *       io.flutter.embedding.engine.FlutterEngine}.
   * </ul>
   *
   * <p>By the end of this method, the {@link android.app.Activity} that was made available in
   * {@link #onAttachedToHostComponent(HostComponentPluginBinding)} is no longer valid. Any
   * references to the associated {@link android.app.Activity} or {@link HostComponentPluginBinding}
   * should be cleared.
   *
   * <p>Any {@code Lifecycle} listeners that were registered in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} or {@link
   * #onReattachedToHostComponentForConfigChanges(HostComponentPluginBinding)} should be
   * deregistered here to avoid a possible memory leak and other side effects.
   */
  void onDetachedFromHostComponent();
}
