// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.android.controller.ActivityController;
import org.robolectric.annotation.Config;

import io.flutter.FlutterInjector;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.lang.IllegalStateException;

@Config(manifest=Config.NONE)
@RunWith(RobolectricTestRunner.class)
public class FlutterInjectorTest {
  @Before
  public void setUp() {
    FlutterInjector.reset();
  }

  @Test
  public void itHasSomeReasonableDefaults() {
    FlutterInjector injector = FlutterInjector.instance();
    assertFalse(injector.isRunningInRobolectricTest());
    assertNotNull(injector.flutterLoader());
  }

  @Test
  public void canPartiallyOverride() {
    FlutterInjector.setInstance(
        new FlutterInjector.Builder().setIsRunningInRobolectricTest(true).build());
    FlutterInjector injector = FlutterInjector.instance();
    assertTrue(injector.isRunningInRobolectricTest());
    assertNotNull(injector.flutterLoader());
  }

  @Test(expected = IllegalStateException.class)
  public void cannotBeChangedOnceRead() {
    FlutterInjector.instance();
    FlutterInjector.setInstance(
        new FlutterInjector.Builder().setIsRunningInRobolectricTest(true).build());
  }
}
