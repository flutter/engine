// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.Context;
import android.support.annotation.NonNull;

import io.flutter.app.FlutterPluginRegistry;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.renderer.FlutterRenderer;

/**
 * A single Flutter execution environment.
 *
 * WARNING: THIS CLASS IS EXPERIMENTAL. DO NOT SHIP A DEPENDENCY ON THIS CODE.
 * IF YOU USE IT, WE WILL BREAK YOU.
 *
 * A {@code FlutterEngine} can execute in the background, or it can be rendered to the screen by
 * using the accompanying {@link FlutterRenderer}.  Rendering can be started and stopped, thus
 * allowing a {@code FlutterEngine} to move from UI interaction to data-only processing and then
 * back to UI interaction.
 *
 * Multiple {@code FlutterEngine}s may exist, execute Dart code, and render UIs within a single
 * Android app.
 *
 * To start running Flutter within this {@code FlutterEngine}, get a reference to this engine's
 * {@link DartExecutor} and then use {@link DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)}.
 * The {@link DartExecutor#executeDartEntrypoint(DartExecutor.DartEntrypoint)} method must not be
 * invoked twice on the same {@code FlutterEngine}.
 *
 * To start rendering Flutter content to the screen, use {@link #getRenderer()} to obtain a
 * {@link FlutterRenderer} and then attach a {@link FlutterRenderer.RenderSurface}.  Consider using
 * a {@link io.flutter.embedding.android.FlutterView} as a {@link FlutterRenderer.RenderSurface}.
 */
public class FlutterEngine {
  private static final String TAG = "FlutterEngine";

  private final FlutterJNI flutterJNI;
  private final FlutterRenderer renderer;
  private final DartExecutor dartExecutor;
  // TODO(mattcarroll): integrate system channels with FlutterEngine
  private final FlutterPluginRegistry pluginRegistry;

  private final EngineLifecycleListener engineLifecycleListener = new EngineLifecycleListener() {
    @SuppressWarnings("unused")
    public void onPreEngineRestart() {
      if (pluginRegistry == null)
        return;
      pluginRegistry.onPreEngineRestart();
    }
  };

  public FlutterEngine(Context context) {
    this.flutterJNI = new FlutterJNI();
    flutterJNI.addEngineLifecycleListener(engineLifecycleListener);
    attachToJni();

    this.dartExecutor = new DartExecutor(flutterJNI);
    this.dartExecutor.onAttachedToJNI();

    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling if possible.
    this.renderer = new FlutterRenderer(flutterJNI);

    this.pluginRegistry = new FlutterPluginRegistry(this, context);
  }

  private void attachToJni() {
    // TODO(mattcarroll): update native call to not take in "isBackgroundView"
    flutterJNI.attachToNative(false);

    if (!isAttachedToJni()) {
      throw new RuntimeException("FlutterEngine failed to attach to its native Object reference.");
    }
  }

  @SuppressWarnings("BooleanMethodIsAlwaysInverted")
  private boolean isAttachedToJni() {
    return flutterJNI.isAttached();
  }

  public void detachFromJni() {
    pluginRegistry.detach();
    dartExecutor.onDetachedFromJNI();
    flutterJNI.removeEngineLifecycleListener(engineLifecycleListener);
    // TODO(mattcarroll): investigate detach vs destroy. document user-cases. update code if needed.
    flutterJNI.detachFromNativeButKeepNativeResources();
  }

  public void destroy() {
    pluginRegistry.destroy();
    dartExecutor.onDetachedFromJNI();
    flutterJNI.removeEngineLifecycleListener(engineLifecycleListener);
    flutterJNI.detachFromNativeAndReleaseResources();
  }

  @NonNull
  public DartExecutor getDartExecutor() {
    return dartExecutor;
  }

  @NonNull
  public FlutterRenderer getRenderer() {
    return renderer;
  }

  // TODO(mattcarroll): propose a robust story for plugin backward compability and future facing API.
  @NonNull
  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }

  /**
   * Lifecycle callbacks for Flutter engine lifecycle events.
   */
  public interface EngineLifecycleListener {
    /**
     * Lifecycle callback invoked before a hot restart of the Flutter engine.
     */
    void onPreEngineRestart();
  }
}
