// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.mouse;

import android.content.Context;
import android.view.PointerIcon;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.systemchannels.MouseCursorChannel;
import java.util.HashMap;

/** Android implementation of the text input plugin. */
public class MouseCursorPlugin {
  @NonNull private final MouseCursorViewDelegate mView;
  @NonNull private final MouseCursorChannel mouseCursorChannel;
  @NonNull private final Context context;
  // A map from each cursor to its cursor object. System icons are cached here
  // as it's first requested.
  @NonNull
  private HashMap<Integer, PointerIcon> cursorObjects = new HashMap<Integer, PointerIcon>();

  public MouseCursorPlugin(
      @NonNull MouseCursorViewDelegate view,
      @NonNull MouseCursorChannel mouseCursorChannel,
      @NonNull Context context) {
    mView = view;
    this.context = context;

    this.mouseCursorChannel = mouseCursorChannel;
    mouseCursorChannel.setMethodHandler(
      new MouseCursorChannel.MouseCursorMethodHandler() {
        @Override
        public void activateSystemCursor(@NonNull Integer shapeCode) {
          mView.setPointerIcon(resolveSystemCursor(shapeCode));
        }
      });
  }

  // Return a cursor object for a cursor. This method guarantees to return a non-null object.
  //
  // This method first tries to find a cached value in cursorObjects.
  //
  // If there is no matching cache, the method tries to create it as a system cursor, which falls
  // back to MouseCursors.basic for all unrecognized values. The value is cached in cursorObjects
  // before being returned.
  private PointerIcon resolveSystemCursor(@NonNull Integer shapeCode) {
    final PointerIcon cached = cursorObjects.get(shapeCode);
    if (cached != null)
      return cached;
    final PointerIcon result = PointerIcon.getSystemIcon(context, shapeCode);
    cursorObjects.put(shapeCode, result);
    return result;
  }

  static Integer _mapShapeCodeToPlatformConstant(int shapeCode) {
    // Shape codes are hard-coded identifiers for system cursors.
    //
    // The shape code values must be kept in sync with flutter's
    // rendering/mouse_cursor.dart
    switch (shapeCode) {
      case /* none */ 0x334c4a:
        return PointerIcon.TYPE_NULL;
      case /* basic */ 0xf17aaa:
        break;
      case /* click */ 0xa8affc:
        return PointerIcon.TYPE_HAND;
      case /* text */ 0x1cb251:
        return PointerIcon.TYPE_TEXT;
      case /* forbidden */ 0x350f9d:
        break;
      case /* grab */ 0x28b91f:
        return PointerIcon.TYPE_GRAB;
      case /* grabbing */ 0x6631ce:
        return PointerIcon.TYPE_GRABBING;
    }
    return PointerIcon.TYPE_ARROW;
  }

  /**
   * Detaches the text input plugin from the platform views controller.
   *
   * <p>The MouseCursorPlugin instance should not be used after calling this.
   */
  public void destroy() {
    mouseCursorChannel.setMethodHandler(null);
  }

  public interface MouseCursorViewDelegate {
    public void setPointerIcon(@NonNull PointerIcon icon);
  }
}
