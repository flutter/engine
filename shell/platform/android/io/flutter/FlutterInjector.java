// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter;

import android.support.annotation.NonNull;
import android.support.annotation.VisibleForTesting;

import io.flutter.embedding.engine.loader.FlutterLoader;

import java.lang.IllegalStateException;

/*
 * This class is a simple dependency injector for the Android part of the Flutter engine.
 *
 * This simple solution is used facilitate testability without bringing in heavier app-development
 * centric dependency injection frameworks such as Guice or Dagger2.
 */
public final class FlutterInjector {

  private static FlutterInjector instance;
  private static boolean accessed;

  /*
   * Use {@link FlutterInjector.Builder} to specify members to be injected via the static
   * {@code FlutterInjector}.
   *
   * This can only be called at the beginning of the program before the {@link #instance()} is
   * accessed.
   */
  public static void setInstance(@NonNull FlutterInjector injector) {
    if (accessed) {
      throw new IllegalStateException("Cannot change the FlutterInjector instance once it's been " +
          "read. If you're trying to dependency inject, be sure to do so at the beginning of " +
          "the program");
    }
    instance = injector;
  }

  /*
   * Retrieve the static instance of the {@code FlutterInjector} to use in your program.
   *
   * Once you access it, you can no longer change the values injected.
   *
   * If no override is provided for the injector, reasonable defaults are provided.
   */
  public static FlutterInjector instance() {
    accessed = true;
    if (instance == null) {
      instance = new Builder().build();
    }
    return instance;
  }

  // This whole class is here to enable testing so to test the thing that lets you test, some degree
  // of hack is needed.
  @VisibleForTesting
  /* Package default */ static void reset() {
    accessed = false;
    instance = null;
  }

  private FlutterInjector(
    boolean isRunningInRobolectricTest,
    @NonNull FlutterLoader flutterLoader
  ) {
    this.isRunningInRobolectricTest = isRunningInRobolectricTest;
    this.flutterLoader = flutterLoader;
  }

  private boolean isRunningInRobolectricTest;
  private FlutterLoader flutterLoader;

  public boolean isRunningInRobolectricTest() {
    return isRunningInRobolectricTest;
  }

  @NonNull
  public FlutterLoader flutterLoader() {
    return flutterLoader;
  }

  /*
   * Builder used to supply a custom FlutterInjector instance to
   * {@link FlutterInjector#setInstance(FlutterInjector)}.
   *
   * Non-overriden values have reasonable defaults.
   */
  public static final class Builder {

    private boolean isRunningInRobolectricTest = false;
    public Builder setIsRunningInRobolectricTest(boolean isRunningInRobolectricTest) {
      this.isRunningInRobolectricTest = isRunningInRobolectricTest;
      return this;
    }

    private FlutterLoader flutterLoader;
    public Builder setFlutterLoader(@NonNull FlutterLoader flutterLoader) {
      this.flutterLoader = flutterLoader;
      return this;
    }

    public FlutterInjector build() {
      if (flutterLoader == null) {
        flutterLoader = new FlutterLoader();
      }

      return new FlutterInjector(isRunningInRobolectricTest, flutterLoader);
    }

  }

}