// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.loader;

import static android.os.Looper.getMainLooper;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.robolectric.Shadows.shadowOf;

import android.app.ActivityManager;
import android.content.Context;
import io.flutter.embedding.engine.FlutterJNI;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

@Config(manifest = Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterLoaderTest {

  @Test
  public void itReportsUninitializedAfterCreating() {
    FlutterLoader flutterLoader = new FlutterLoader();
    assertFalse(flutterLoader.initialized());
  }

  @Test
  public void itReportsInitializedAfterInitializing() {
    FlutterJNI.FlutterJNILoader mockFlutterJNILoader = mock(FlutterJNI.FlutterJNILoader.class);
    FlutterLoader flutterLoader = new FlutterLoader(mockFlutterJNILoader);

    assertFalse(flutterLoader.initialized());
    flutterLoader.startInitialization(RuntimeEnvironment.application);
    flutterLoader.ensureInitializationComplete(RuntimeEnvironment.application, null);
    shadowOf(getMainLooper()).idle();
    assertTrue(flutterLoader.initialized());
    verify(mockFlutterJNILoader, times(1)).loadLibrary();

    ActivityManager activityManager =
        (ActivityManager) RuntimeEnvironment.application.getSystemService(Context.ACTIVITY_SERVICE);
    verify(mockFlutterJNILoader, times(1))
        .nativeInit(
            eq(RuntimeEnvironment.application),
            any(),
            anyString(),
            anyString(),
            anyString(),
            anyLong(),
            eq(activityManager.getLargeMemoryClass()));
  }
}
