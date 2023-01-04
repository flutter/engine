// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.shim;

import androidx.annotation.NonNull;
import io.flutter.Log;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.host.HostComponentAware;
import io.flutter.embedding.engine.plugins.host.HostComponentPluginBinding;
import io.flutter.plugin.common.PluginRegistry;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * A {@link PluginRegistry} that is shimmed to let old plugins use the new Android embedding and
 * plugin API behind the scenes.
 *
 * <p>The following is an example usage of {@code ShimPluginRegistry} within a {@code
 * FlutterActivity}:
 *
 * <pre>
 * // Create the FlutterEngine that will back the Flutter UI.
 * FlutterEngineGroup group = new FlutterEngineGroup(context);
 * FlutterEngine flutterEngine = group.createAndRunDefaultEngine(context);
 *
 * // Create a ShimPluginRegistry and wrap the FlutterEngine with the shim.
 * ShimPluginRegistry shimPluginRegistry = new ShimPluginRegistry(flutterEngine, platformViewsController);
 *
 * // Use the GeneratedPluginRegistrant to add every plugin that's in the pubspec.
 * GeneratedPluginRegistrant.registerWith(shimPluginRegistry);
 * </pre>
 */
public class ShimPluginRegistry implements PluginRegistry {
  private static final String TAG = "ShimPluginRegistry";

  private final FlutterEngine flutterEngine;
  private final Map<String, Object> pluginMap = new HashMap<>();
  private final ShimRegistrarAggregate shimRegistrarAggregate;

  public ShimPluginRegistry(@NonNull FlutterEngine flutterEngine) {
    this.flutterEngine = flutterEngine;
    this.shimRegistrarAggregate = new ShimRegistrarAggregate();
    this.flutterEngine.getPlugins().add(shimRegistrarAggregate);
  }

  @Override
  @NonNull
  public Registrar registrarFor(@NonNull String pluginKey) {
    Log.v(TAG, "Creating plugin Registrar for '" + pluginKey + "'");
    if (pluginMap.containsKey(pluginKey)) {
      throw new IllegalStateException("Plugin key " + pluginKey + " is already in use");
    }
    pluginMap.put(pluginKey, null);
    ShimRegistrar registrar = new ShimRegistrar(pluginKey, pluginMap);
    shimRegistrarAggregate.addPlugin(registrar);
    return registrar;
  }

  @Override
  public boolean hasPlugin(@NonNull String pluginKey) {
    return pluginMap.containsKey(pluginKey);
  }

  @Override
  @SuppressWarnings("unchecked")
  public <T> T valuePublishedByPlugin(@NonNull String pluginKey) {
    return (T) pluginMap.get(pluginKey);
  }

  /**
   * Aggregates all {@link ShimRegistrar}s within one single {@link FlutterPlugin}.
   *
   * <p>The reason we need this aggregate is because the new embedding uniquely identifies plugins
   * by their plugin class, but the plugin shim system represents every plugin with a {@link
   * ShimRegistrar}. Therefore, every plugin we would register after the first plugin, would
   * overwrite the previous plugin, because they're all {@link ShimRegistrar} instances.
   *
   * <p>{@code ShimRegistrarAggregate} multiplexes {@link FlutterPlugin} and {@link
   * HostComponentAware} calls so that we can register just one {@code ShimRegistrarAggregate} with
   * a {@link FlutterEngine}, while forwarding the relevant plugin resources to any number of {@link
   * ShimRegistrar}s within this {@code ShimRegistrarAggregate}.
   */
  private static class ShimRegistrarAggregate implements FlutterPlugin, HostComponentAware {
    private final Set<ShimRegistrar> shimRegistrars = new HashSet<>();
    private FlutterPluginBinding flutterPluginBinding;
    private HostComponentPluginBinding HostComponentPluginBinding;

    public void addPlugin(@NonNull ShimRegistrar shimRegistrar) {
      shimRegistrars.add(shimRegistrar);

      if (flutterPluginBinding != null) {
        shimRegistrar.onAttachedToEngine(flutterPluginBinding);
      }
      if (HostComponentPluginBinding != null) {
        shimRegistrar.onAttachedToHostComponent(HostComponentPluginBinding);
      }
    }

    @Override
    public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
      flutterPluginBinding = binding;
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onAttachedToEngine(binding);
      }
    }

    @Override
    public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onDetachedFromEngine(binding);
      }
      flutterPluginBinding = null;
      HostComponentPluginBinding = null;
    }

    @Override
    public void onAttachedToHostComponent(@NonNull HostComponentPluginBinding binding) {
      HostComponentPluginBinding = binding;
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onAttachedToHostComponent(binding);
      }
    }

    @Override
    public void onDetachedFromHostComponentForConfigChanges() {
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onDetachedFromHostComponent();
      }
      HostComponentPluginBinding = null;
    }

    @Override
    public void onReattachedToHostComponentForConfigChanges(
        @NonNull HostComponentPluginBinding binding) {
      HostComponentPluginBinding = binding;
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onReattachedToHostComponentForConfigChanges(binding);
      }
    }

    @Override
    public void onDetachedFromHostComponent() {
      for (ShimRegistrar shimRegistrar : shimRegistrars) {
        shimRegistrar.onDetachedFromHostComponent();
      }
      HostComponentPluginBinding = null;
    }
  }
}
