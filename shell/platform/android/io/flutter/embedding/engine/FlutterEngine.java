// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

/**
 * WARNING: THIS CLASS IS EXPERIMENTAL. DO NOT SHIP A DEPENDENCY ON THIS CODE.
 * IF YOU USE IT, WE WILL BREAK YOU.
 */
public class FlutterEngine {
  // TODO(mattcarroll): bring in FlutterEngine implementation in future PR

  /**
   * Lifecycle callbacks for Flutter engine lifecycle events.
   */
  public interface EngineLifecycleListener {
    /**
     * Lifecycle callback invoked after a hot restart of the Flutter engine.
     */
    void onPreEngineRestart();
  }
}
