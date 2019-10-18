// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.mouse;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.View;
import android.view.PointerIcon;

import java.util.HashMap;

import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.systemchannels.MouseCursorChannel;

/**
 * Manages mouse cursor of the mouse device.
 *
 * Android only allows one pointer device.
 *
 * Each cursor has a representation, which is an integer, and has an implementation, which is a
 * {@code PointerIcon} object. System cursors, which are the constants defined in
 * {@code MouseCursors}, can be directly used, while you can also use
 * {@code MouseCursorController.registerCursor} to define custom cursors or overwrite system
 * cursors. Unregistered cursors will fallback to {@code MouseCursors.basic}.
 *
 * This class only communicates in Flutter's cursor constants, which means it doesn't recognize the
 * constants defined in {@code PointerIcon}.
 */
public class MouseCursors {
  // Constants here must be kept in sync with mouse_cursors.dart

  /* Displays no cursor at the pointer.
   */
  public final static int none = 0x334c4a4c;

  /* The platform-dependent basic cursor. Typically an arrow.
   */
  public final static int basic = 0xf17aaabc;

  /* A cursor that indicates a link or other clickable object that is not obvious enough
   * otherwise. Typically the shape of a pointing hand.
   */
  public final static int click = 0xa8affc08;

  /* A cursor that indicates a selectable text. Typically the shape of an I-beam.
   */
  public final static int text = 0x1cb251ec;

  /* A cursor that indicates that the intended action is not permitted. Typically the shape
   * of a circle with a diagnal line.
   */
  public final static int no = 0x7fa3b767;

  /* A cursor that indicates something that can be dragged. Typically the shape of an open
   * hand.
   */
  public final static int grab = 0x28b91f80;

  /* A cursor that indicates something that is being dragged. Typically the shape of a closed
   * hand.
   */
  public final static int grabbing = 0x6631ce3e;

  public static int resolveSystemCursorConstant(@NonNull int cursor) {
    switch (cursor) {
      case none:
        return PointerIcon.TYPE_NULL;
      case basic:
        break;
      case click:
        return PointerIcon.TYPE_HAND;
      case text:
        return PointerIcon.TYPE_TEXT;
      case no:
        break;
      case grab:
        return PointerIcon.TYPE_GRAB;
      case grabbing:
        return PointerIcon.TYPE_GRABBING;
    }
    return PointerIcon.TYPE_ARROW;
  }
}
