// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.engine.dart.DartExecutor.DartEntrypoint;
import java.util.ArrayList;
import java.util.List;

/**
 * This class is experimental. Please do not ship production code using it.
 *
 * <p>Represents a collection of {@link io.flutter.embedding.engine.FlutterEngine}s who share
 * resources among each other to allow them to be created faster and with less memory than calling
 * the {@link io.flutter.embedding.engine.FlutterEngine}'s constructor multiple times.
 *
 * <p>When creating or recreating the first {@link io.flutter.embedding.engine.FlutterEngine} in the
 * FlutterEngineGroup, the behavior is the same as creating a {@link
 * io.flutter.embedding.engine.FlutterEngine} via its constructor. When subsequent {@link
 * io.flutter.embedding.engine.FlutterEngine}s are created, resources from an existing living {@link
 * io.flutter.embedding.engine.FlutterEngine} is re-used.
 *
 * <p>Deleting a FlutterEngineGroup doesn't invalidate its existing {@link
 * io.flutter.embedding.engine.FlutterEngine}s, but it eliminates the possibility to create more
 * {@link io.flutter.embedding.engine.FlutterEngine}s in that group.
 */
public class FlutterEngineGroup {

  public FlutterEngineGroup(@NonNull Context context) {
    this.context = context;
  }

  private final Context context;
  private final List<FlutterEngine> activeEngines = new ArrayList<>();

  public FlutterEngine createAndRunDefaultEngine() {
    return createAndRunEngine(null);
  }

  public FlutterEngine createAndRunEngine(@Nullable DartEntrypoint dartEntrypoint) {
    FlutterEngine engine = null;
    if (activeEngines.size() == 0) {
      engine = new FlutterEngine(context);
    }

    if (dartEntrypoint == null) {
      dartEntrypoint = DartEntrypoint.createDefault();
    }

    if (activeEngines.size() == 0) {
      engine.getDartExecutor().executeDartEntrypoint(dartEntrypoint);
    } else {
      engine = activeEngines.get(0).spawn(dartEntrypoint);
    }

    activeEngines.add(engine);

    final FlutterEngine engineToCleanUpOnDestroy = engine;
    engine.addEngineLifecycleListener(
        new FlutterEngine.EngineLifecycleListener() {

          @Override
          public void onPreEngineRestart() {
            // No-op. Not interested.
          }

          @Override
          public void onEngineDestroy() {
            activeEngines.remove(engineToCleanUpOnDestroy);
          }
        });
    return engine;
  }
}
