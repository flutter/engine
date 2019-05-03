// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins;

import android.arch.lifecycle.Lifecycle;
import android.content.Context;
import android.support.annotation.NonNull;

import io.flutter.embedding.engine.FlutterEngine;

/**
 * Interface to be implemented by all Flutter plugins.
 * <p>
 * A Flutter plugin allows Flutter developers to interact with a host platform, e.g., Android and
 * iOS, via Dart code. A Flutter plugin includes platform code, as well as Dart code. A plugin
 * author is responsible for setting up an appropriate {@link io.flutter.plugin.common.MethodChannel}
 * to communicate between platform code and Dart code.
 * <p>
 * A Flutter plugin has a lifecycle. A Flutter plugin must be registered with a given instance of a
 * {@link FlutterEngine}. When a {@code FlutterPlugin} is registered with a {@link FlutterEngine},
 * the plugin's {@link #onAttachedToEngine(FlutterPluginBinding)} is invoked. When the
 * {@code FlutterPlugin} is detached from the {@link FlutterEngine}, the plugin's
 * {@link #onDetachedFromEngine(FlutterPluginBinding)} is invoked.
 * <p>
 * A {@code FlutterPlugin} is permitted to interact with its {@link FlutterPluginBinding} between
 * invocations of {@link #onAttachedToEngine(FlutterPluginBinding)} and
 * {@link #onDetachedFromEngine(FlutterPluginBinding)}. However, the {@link FlutterPluginBinding}
 * should not be cached or accessed after the completion of
 * {@link #onDetachedFromEngine(FlutterPluginBinding)}.
 * <p>
 * The Android side of a Flutter plugin can be thought of as applying itself to a {@link FlutterEngine},
 * which can be retrieved from a {@link FlutterPluginBinding} in
 * {@link #onAttachedToEngine(FlutterPluginBinding)}. To register a
 * {@link io.flutter.plugin.common.MethodChannel}, use {@link FlutterEngine}'s
 * {@link io.flutter.embedding.engine.dart.DartExecutor}.
 * <p>
 * An Android Flutter plugin may require access to app resources or other artifacts that can only
 * be retrieved through a {@link Context}. Therefore, the application's {@link Context} is made
 * available via {@link FlutterPluginBinding#getApplicationContext()}.
 * <p>
 * TODO(mattcarroll): explain ActivityAware when it's added to the new plugin API surface.
 */
public interface FlutterPlugin {

  /**
   * This {@code FlutterPlugin} has been associated with a {@link FlutterEngine} instance.
   * <p>
   * Relevant resources that this {@code FlutterPlugin} may need are provided via the {@code binding}.
   * The {@code binding} may be cached and referenced until
   * {@link #onDetachedFromEngine(FlutterPluginBinding)} is invoked and returns.
   */
  void onAttachedToEngine(@NonNull FlutterPluginBinding binding);

  /**
   * This {@code FlutterPlugin} has been removed from a {@link FlutterEngine} instance.
   * <p>
   * The {@code binding} passed to this method is the same instance that was passed in
   * {@link #onAttachedToEngine(FlutterPluginBinding)}. It is provided again in this method as a
   * convenience. The {@code binding} may be referenced during the execution of this method, but
   * it must not be cached or referenced after this method returns.
   * <p>
   * {@code FlutterPlugin}s should release all resources in this method.
   */
  void onDetachedFromEngine(@NonNull FlutterPluginBinding binding);

  /**
   * Resources made available to all plugins registered with a given {@link FlutterEngine}.
   * <p>
   * The {@code FlutterPluginBinding}'s {@code flutterEngine} refers to the {@link FlutterEngine}
   * that the associated {@code FlutterPlugin} is intended to apply to. For example, if a
   * {@code FlutterPlugin} needs to setup a communication channel with its associated Flutter app,
   * that can be done by wrapping a {@code MethodChannel} around
   * {@link FlutterEngine#getDartExecutor()}.
   * <p>
   * A {@link FlutterEngine} may move from foreground to background, from an {@code Activity} to
   * a {@code Service}, and {@code FlutterPluginBinding}'s {@code lifecycle} generalizes those
   * lifecycles so that a {@code FlutterPlugin} can react to lifecycle events without being
   * concerned about which Android Component is currently holding the {@link FlutterEngine}.
   * TODO(mattcarroll): add info about ActivityAware and ServiceAware for plugins that care.
   */
  class FlutterPluginBinding {
        private final Context applicationContext;
        private final FlutterEngine flutterEngine;
        private final Lifecycle lifecycle;

        public FlutterPluginBinding(
            @NonNull Context applicationContext,
            @NonNull FlutterEngine flutterEngine,
            @NonNull Lifecycle lifecycle
        ) {
            this.applicationContext = applicationContext;
            this.flutterEngine = flutterEngine;
            this.lifecycle = lifecycle;
        }

        @NonNull
        public Context getApplicationContext() {
            return applicationContext;
        }

        @NonNull
        public FlutterEngine getFlutterEngine() {
            return flutterEngine;
        }

        @NonNull
        public Lifecycle getLifecycle() {
            return lifecycle;
        }
    }
}
