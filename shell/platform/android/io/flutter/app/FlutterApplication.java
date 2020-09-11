// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.app;

import android.app.Activity;
import android.content.Context;
import android.app.Application;
import androidx.annotation.CallSuper;
import com.google.android.play.core.splitcompat.SplitCompat;
import io.flutter.FlutterInjector;

/**
 * Flutter implementation of {@link android.app.Application}, managing application-level global
 * initializations.
 *
 * <p>Using this {@link android.app.Application} is not required when using APIs in the package
 * {@code io.flutter.embedding.android} since they self-initialize on first use.
 */
public class FlutterApplication extends Application {
  @Override
  @CallSuper
  public void onCreate() {
    super.onCreate();
    FlutterInjector.instance().flutterLoader().startInitialization(this);
  }

  private Activity mCurrentActivity = null;

  public Activity getCurrentActivity() {
    return mCurrentActivity;
  }

  public void setCurrentActivity(Activity mCurrentActivity) {
    this.mCurrentActivity = mCurrentActivity;
  }

  // This override allows split dynamic feature modules to work.
  @Override
  protected void attachBaseContext(Context base) {
    super.attachBaseContext(base);
    // Emulates installation of future on demand modules using SplitCompat.
    SplitCompat.install(this);
  }
}
