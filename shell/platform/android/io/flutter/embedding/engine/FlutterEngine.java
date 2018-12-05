// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.Context;
import android.content.res.Resources;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import io.flutter.app.FlutterPluginRegistry;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.systemchannels.SystemChannels;
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
  private final SystemChannels systemChannels;
  private final FlutterPluginRegistry pluginRegistry;
  private long nativeObjectReference;

  public FlutterEngine(Context context) {
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

    this.dartExecutor = new DartExecutor(flutterJNI, nativeObjectReference);
    this.dartExecutor.onAttachedToJNI();

    this.systemChannels = new SystemChannels(dartExecutor);

    // TODO(mattcarroll): FlutterRenderer is temporally coupled to attach(). Remove that coupling if possible.
    this.renderer = new FlutterRenderer(flutterJNI, nativeObjectReference);

    this.pluginRegistry = new FlutterPluginRegistry(this, context);
  }

  private void attachToJni() {
    // TODO(mattcarroll): update native call to not take in "isBackgroundView"
    nativeObjectReference = flutterJNI.nativeAttach(flutterJNI, false);

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
    flutterJNI.nativeDestroy(nativeObjectReference);
    nativeObjectReference = 0;
  }

  @NonNull
  public DartExecutor getDartExecutor() {
    return dartExecutor;
  }

  @NonNull
  public SystemChannels getSystemChannels() {
    return systemChannels;
  }

  @NonNull
  public FlutterRenderer getRenderer() {
    return renderer;
  }

  @NonNull
  public FlutterPluginRegistry getPluginRegistry() {
    return pluginRegistry;
  }


  // TODO(mattcarroll): what does this callback actually represent? nothing uses it...
  // Called by native to notify when the engine is restarted (cold reload).
  @SuppressWarnings("unused")
  private void onPreEngineRestart() {
    if (pluginRegistry == null)
      return;
    pluginRegistry.onPreEngineRestart();
  }

  /** Callbacks triggered by the C++ layer in response to Engine lifecycle events. */
  public interface EngineHandler {
    /** Called when the engine is restarted. This happens during hot restart. */
    void onPreEngineRestart();
  }
}
