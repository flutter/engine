// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.host;

import androidx.annotation.NonNull;

/**
 * {@link io.flutter.embedding.engine.plugins.FlutterPlugin} that is interested in {@link
 * HostComponent} lifecycle events related to a {@link io.flutter.embedding.engine.FlutterEngine}
 * running within the given {@link HostComponent}.
 */
public interface HostComponentAware {
  /**
   * This {@code HostComponentAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin} is
   * now associated with an {@link HostComponent}.
   *
   * <p>This method can be invoked in 1 of 2 situations:
   *
   * <ul>
   *   <li>This {@code HostComponentAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin}
   *       was just added to a {@link io.flutter.embedding.engine.FlutterEngine} that was already
   *       connected to a running {@link HostComponent}.
   *   <li>This {@code ActivityAware} {@link io.flutter.embedding.engine.plugins.FlutterPlugin} was
   *       already added to a {@link io.flutter.embedding.engine.FlutterEngine} and that {@link
   *       io.flutter.embedding.engine.FlutterEngine} was just connected to an {@link
   *       HostComponent}.
   * </ul>
   *
   * <p>The given {@link HostComponentPluginBinding} contains {@link android.app.Activity}-related
   * references that an {@code HostComponentAware} {@link
   * io.flutter.embedding.engine.plugins.FlutterPlugin} may require, such as a reference to the
   * actual {@link HostComponent} in question. The {@link HostComponentPluginBinding} may be
   * referenced until either {@link #onDetachedFromHostComponentForConfigChanges()} or {@link
   * #onDetachedFromHostComponent()} is invoked. At the conclusion of either of those methods, the
   * binding is no longer valid. Clear any references to the binding or its resources, and do not
   * invoke any further methods on the binding or its resources.
   */
  void onAttachedToHostComponent(@NonNull HostComponentPluginBinding binding);

  /**
   * The {@link HostComponent} that was attached and made available in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} has been detached from this {@code
   * HostComponentAware}'s {@link io.flutter.embedding.engine.FlutterEngine} for the purpose of
   * processing a configuration change.
   *
   * <p>By the end of this method, the {@link HostComponent} that was made available in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} is no longer valid. Any references to
   * the associated {@link HostComponent} or {@link HostComponentPluginBinding} should be cleared.
   *
   * <p>This method should be quickly followed by {@link
   * #onReattachedToHostComponentForConfigChanges(HostComponentPluginBinding)}, which signifies that
   * a new {@link HostComponent} has been created with the new configuration options. That method
   * provides a new {@link HostComponentPluginBinding}, which references the newly created and
   * associated {@link HostComponent}.
   *
   * <p>Any {@code Lifecycle} listeners that were registered in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} should be deregistered here to avoid a
   * possible memory leak and other side effects.
   */
  void onDetachedFromHostComponentForConfigChanges();

  /**
   * This plugin and its {@link io.flutter.embedding.engine.FlutterEngine} have been re-attached to
   * an {@link HostComponent} after the {@link HostComponent} was recreated to handle configuration
   * changes.
   *
   * <p>{@code binding} includes a reference to the new instance of the {@link HostComponent}.
   * {@code binding} and its references may be cached and used from now until either {@link
   * #onDetachedFromHostComponentForConfigChanges()} or {@link #onDetachedFromHostComponent()} is
   * invoked. At the conclusion of either of those methods, the binding is no longer valid. Clear
   * any references to the binding or its resources, and do not invoke any further methods on the
   * binding or its resources.
   */
  void onReattachedToHostComponentForConfigChanges(@NonNull HostComponentPluginBinding binding);

  /**
   * This plugin has been detached from an {@link HostComponent}.
   *
   * <p>Detachment can occur for a number of reasons.
   *
   * <ul>
   *   <li>The app is no longer visible and the {@link HostComponent} instance has been destroyed.
   *   <li>The {@link io.flutter.embedding.engine.FlutterEngine} that this plugin is connected to
   *       has been detached from its {@link io.flutter.embedding.android.FlutterView}.
   *   <li>This {@code ActivityAware} plugin has been removed from its {@link
   *       io.flutter.embedding.engine.FlutterEngine}.
   * </ul>
   *
   * <p>By the end of this method, the {@link HostComponent} that was made available in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} is no longer valid. Any references to
   * the associated {@link HostComponent} or {@link HostComponentPluginBinding} should be cleared.
   *
   * <p>Any {@code Lifecycle} listeners that were registered in {@link
   * #onAttachedToHostComponent(HostComponentPluginBinding)} or {@link
   * #onReattachedToHostComponentForConfigChanges(HostComponentPluginBinding)} should be
   * deregistered here to avoid a possible memory leak and other side effects.
   */
  void onDetachedFromHostComponent();
}
