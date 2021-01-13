// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine;

import java.util.List;

/**
 * This class is experimental. Please do not ship production code using it.
 *
 * Represents a collection of {@link FlutterEngine}s who share resources among
 * each other to allow them to be created faster and with less memory than
 * calling the {@link FlutterEngine}'s constructor multiple times.
 *
 * When creating or recreating the first {@link FlutterEngine} in the
 * FlutterEngineGroup, the behavior is the same as creating a
 * {@link FlutterEngine} via its constructor. When subsequent
 * {@link FlutterEngine}s are created, resources from an existing living
 * {@link FlutterEngine} is re-used.
 *
 * Deleting a FlutterEngineGroup doesn't invalidate its existing
 * {@link FlutterEngine}s, but it eliminates the possibility to create more
 * {@link FlutterEngine}s in that group.
 */
public class FlutterEngineGroup {

  private List<FlutterEngine> activeEngines = ArrayList<>();

}
