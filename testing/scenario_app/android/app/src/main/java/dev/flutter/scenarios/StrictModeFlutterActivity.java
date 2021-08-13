// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package dev.flutter.scenarios;

import android.os.Bundle;
import android.os.StrictMode;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.Log;
import io.flutter.embedding.android.FlutterActivity;
import io.flutter.embedding.android.RenderMode;
import io.flutter.embedding.engine.FlutterEngine;

// TODO: Trigger this activity.
// https://github.com/flutter/flutter/issues/60635
public class StrictModeFlutterActivity extends FlutterActivity {
  @Nullable public FlutterEngine flutterEngine;

  @Override
  protected void onCreate(@Nullable Bundle savedInstanceState) {
    Log.setLogLevel(Log.VERBOSE);
    StrictMode.setThreadPolicy(
        new StrictMode.ThreadPolicy.Builder()
            .detectDiskReads()
            .detectDiskWrites()
            .penaltyDeath()
            .build());
    StrictMode.setVmPolicy(
        new StrictMode.VmPolicy.Builder()
            .detectLeakedClosableObjects()
            .penaltyLog()
            .penaltyDeath()
            .build());
    super.onCreate(savedInstanceState);
  }

  @NonNull
  @Override
  public RenderMode getRenderMode() {
    return RenderMode.texture;
  }

  @Override
  public void configureFlutterEngine(@NonNull FlutterEngine flutterEngine) {
    super.configureFlutterEngine(flutterEngine);
    this.flutterEngine = flutterEngine;
  }
}
