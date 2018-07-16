// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

/**
 * An interface for a callback which is invoked once a new isolate has been
 * started by the engine.
 */
public interface FlutterIsolateStartedEvent {
  public void onStarted(boolean success);
}
