// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.activity;

import io.flutter.embedding.engine.plugins.host.HostComponentPluginBinding;

/**
 * Binding that gives {@link ActivityAware} plugins access to an associated {@link
 * android.app.Activity} and the {@link android.app.Activity}'s lifecycle methods.
 *
 * <p>To obtain an instance of an {@code ActivityPluginBinding} in a Flutter plugin, implement the
 * {@link ActivityAware} interface. A binding is provided in {@link
 * ActivityAware#onAttachedToActivity(ActivityPluginBinding)} and {@link
 * ActivityAware#onReattachedToActivityForConfigChanges(ActivityPluginBinding)}.
 */
public interface ActivityPluginBinding extends HostComponentPluginBinding {

  interface OnSaveInstanceStateListener
      extends HostComponentPluginBinding.OnSaveInstanceStateListener {}
}
