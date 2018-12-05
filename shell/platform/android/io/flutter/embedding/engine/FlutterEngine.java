// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.Context;
import android.content.res.Resources;
import android.support.annotation.NonNull;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.legacy.FlutterPluginRegistry;
import io.flutter.view.FlutterRunArguments;

/**
 * A single Flutter execution environment.
 *
 * A {@code FlutterEngine} can execute in the background, or it can be rendered to the screen by
 * using the accompanying {@link FlutterRenderer}.  Rendering can be started and stopped, thus
 * allowing a {@code FlutterEngine} to move from UI interaction to data-only processing and then
 * back to UI interaction.
 *
 * To start running Flutter within this {@code FlutterEngine}, get a reference to this engine's
 * {@link DartExecutor} and then use {@link DartExecutor#runFromBundle(FlutterRunArguments)}.
 * The {@link DartExecutor#runFromBundle(FlutterRunArguments)} method must not be invoked twice on the same
 * {@code FlutterEngine}.
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
  private final FlutterPluginRegistry pluginRegistry;
  private long nativeObjectReference;
  private boolean isBackgroundView; // TODO(mattcarroll): rename to something without "view"

  public FlutterEngine(
      Context context,
      Resources resources,
      boolean isBackgroundView
  ) {
    this.isBackgroundView = isBackgroundView;

    this.flutterJNI = new FlutterJNI();
    flutterJNI.setEngineHandler(new EngineHandler() {
      @SuppressWarnings("unused")
      public void onPreEngineRestart() {
        if (pluginRegistry == null)
          return;
        pluginRegistry.onPreEngineRestart();
      }
    });
    attachToJni();

    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling if possible.
    this.renderer = new FlutterRenderer(flutterJNI, nativeObjectReference);

    this.dartExecutor = new DartExecutor(flutterJNI, nativeObjectReference, resources);
    this.dartExecutor.onAttachedToJNI();

    this.pluginRegistry = new FlutterPluginRegistry(this, dartExecutor, context);
  }

  private void attachToJni() {
    // TODO(mattcarroll): what impact does "isBackgroundView' have?
    nativeObjectReference = flutterJNI.nativeAttach(flutterJNI, isBackgroundView);

    if (!isAttachedToJni()) {
      throw new RuntimeException("FlutterEngine failed to attach to its native Object reference.");
    }
  }

  @SuppressWarnings("BooleanMethodIsAlwaysInverted")
  private boolean isAttachedToJni() {
    return nativeObjectReference != 0;
  }

  public void detachFromJni() {
    pluginRegistry.detach();
    dartExecutor.onDetachedFromJNI();
    // TODO(mattcarroll): why do we have a nativeDetach() method? can we get rid of this?
    flutterJNI.nativeDetach(nativeObjectReference);
  }

  public void destroy() {
    pluginRegistry.destroy();
    dartExecutor.stop();

    flutterJNI.nativeDestroy(nativeObjectReference);
    nativeObjectReference = 0;
  }

  @NonNull
  public FlutterRenderer getRenderer() {
    return renderer;
  }

  @NonNull
  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }

  @NonNull
  public DartExecutor getDartExecutor() {
    return dartExecutor;
  }

  /** Callbacks triggered by the C++ layer in response to Engine lifecycle events. */
  public interface EngineHandler {
    /** Called when the engine is restarted. This happens during hot restart. */
    void onPreEngineRestart();
  }
}
