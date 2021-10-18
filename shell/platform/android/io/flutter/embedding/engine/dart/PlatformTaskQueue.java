// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.dart;

import android.os.Handler;
import android.os.Looper;
import androidx.annotation.NonNull;
import io.flutter.plugin.common.BinaryMessenger;

public class PlatformTaskQueue implements BinaryMessenger.TaskQueue {
  @NonNull private final Handler handler = new Handler(Looper.getMainLooper());

  @Override
  public void dispatch(@NonNull Runnable runnable) {
    handler.post(runnable);
  }
}
