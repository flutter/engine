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
import io.flutter.plugin.mouse.MouseCursors;

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
public class MouseCursorController {
  private static final String TAG = "MouseCursorController";

  /**
   * Constructs a {@code MouseCursorController} that controls the cursor on the given {@code view}.
   *
   * The given {@code dartExecutor} is used to construct the method channel with which the
   * platform communicates with the framework.
   *
   * The given {@code context} is used to create system cursor object. Usually it's the current
   * activity.
   */
  public MouseCursorController(
      @NonNull View view,
      @NonNull DartExecutor dartExecutor,
      @NonNull Context context
  ) {
    channel = new MouseCursorChannel(dartExecutor);
    channel.setMethodHandler(channelHandler);
    this.view = view;
    this.context = context;
    currentCursor = MouseCursors.basic;
  }

  // The system channel used to communicate with the framework about mouse cursor.
  @NonNull
  private MouseCursorChannel channel;

  // The target application view.
  @NonNull
  private View view;

  // The target application context.
  @NonNull
  private Context context;

  // The current cursor of the device. It starts with the basic cursor.
  @NonNull
  private int currentCursor;

  // A map from each cursor to its cursor object.
  // System icons are cached here as it's first requested. Registering a custom icons also stores
  // the object here. If a custom icon is not registered when it's first requested, then
  // {@code MouseCursors.basic} will still be stored here.
  @NonNull
  private HashMap<Integer, PointerIcon> cursorObjects = new HashMap<Integer, PointerIcon>();

  @NonNull
  private final MouseCursorChannel.MouseCursorMethodHandler channelHandler = new MouseCursorChannel.MouseCursorMethodHandler() {
    public void setCursor(int cursor) {
      if (currentCursor != cursor) {
        currentCursor = cursor;
        int systemCursorConstant = MouseCursors.resolveSystemCursorConstant(cursor);
        view.setPointerIcon(resolveCursor(cursor));
      }
    }
  };

  /**
   * Registers a representation {@code cursor} with an implementation {@code cursorObject}.
   *
   * This method not only affects future requests, but also updates the current device if
   * applicable.
   */
  public void registerCursor(@NonNull int cursor, @NonNull PointerIcon cursorObject) {
    cursorObjects.put(new Integer(cursor), cursorObject);
    if (currentCursor == cursor)
      view.setPointerIcon(cursorObject);
  }

  // Return a cursor object for a cursor. This method guarantees to return a non-null object.
  //
  // This method first tries to find a cached value in cursorObjects.
  //
  // If there is no matching cache, the method tries to create it as a system cursor, which falls
  // back to MouseCursors.basic for all unrecognized values. The value is cached in cursorObjects
  // before being returned.
  private PointerIcon resolveCursor(@NonNull int cursor) {
    final PointerIcon cached = cursorObjects.get(new Integer(cursor));
    if (cached != null)
      return cached;
    final int systemCursorConstant = MouseCursors.resolveSystemCursorConstant(cursor);
    final PointerIcon result = PointerIcon.getSystemIcon(context, systemCursorConstant);
    cursorObjects.put(new Integer(cursor), result);
    return result;
  }
}
