// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.app;

import android.content.Context;
import androidx.annotation.CallSuper;
import androidx.multidex.MultiDex;

/**
 * An extension of {@link FlutterApplication} that supports MultiDex for apps that support minSdk 20
 * and below.
 *
 * <p>Use this class as the Flutter application in AndroidManifest.xml in conjuntion with the
 * --multi-dex flag to enable multi dex support for legacy android API versions. If the minSdk
 * version is 21 or above, multi dex is natively supported and this is not needed.
 */
public class FlutterMultiDexApplication extends FlutterApplication {
  @Override
  @CallSuper
  protected void attachBaseContext(Context base) {
    super.attachBaseContext(base);
    MultiDex.install(this);
  }
}
