// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.common;

import android.content.Context;
import io.flutter.view.FlutterMain;
import io.flutter.view.FlutterView;

/**
 * DartRunner is an API exposed to Flutter plugins for running Dart code in
 * a background Dart Isolate.
 */
public class HeadlessDartRunner {
  private FlutterView mFlutterView;
  private String mAppBundlePath;
  private long mSendPort;

  public HeadlessDartRunner(Context context) {
    FlutterMain.ensureInitializationComplete(context, null);
    // TODO(zra): Create something lighter-weight than FlutterView here.
    mFlutterView = new FlutterView(context);
    mAppBundlePath = FlutterMain.findAppBundlePath(context);
    mSendPort = 0;
  }

  /**
   * Returns the SendPort ID for the RawReceivePort passed to the first
   * invocation of run() on this DartRunner.
   */
  public long sendPort() {
    return mSendPort;
  }

  /**
   * Runs the top-level Dart function called `entrypoint`.
   *
   * Multiple calls to run() on the same DartRunner will reuse the same
   * Dart Isolate. In the first call to run() on a DartRunner, the Dart
   * entrypoint will be passed a single-element array containing a
   * RawReceivePort. The send port for that receive port is given by
   * the `sendPort()` method. In subsequent calls the entrypoint is passed
   * `null`.
   */
  public void run(String entrypoint) {
    if (mFlutterView == null) return;
    final boolean reuseIsolate = true;
    final boolean createSendPort = mSendPort == 0;
    final long port = mFlutterView.runFromBundle(
        mAppBundlePath, null, entrypoint, reuseIsolate, createSendPort);
    mSendPort = createSendPort ? port : mSendPort;
  }

  /**
   * Destroys the DartRunner. Subsequent calls to the run() method will
   * silently fail.
   */
  public void destroy() {
    if (mFlutterView == null) return;
    mFlutterView.destroy();
    mFlutterView = null;
    mAppBundlePath = null;
  }
}
