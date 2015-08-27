// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/pointer_router.dart';
import 'package:sky/gestures/arena.dart';
import 'package:sky/theme/view_configuration.dart';

typedef void GestureTapCallback();

enum TapState {
  ready,
  possible,
  defunct
}

sky.Point _getPoint(sky.PointerEvent event) {
  return new sky.Point(event.x, event.y);
}

class TapGestureRecognizer extends GestureArenaMember {
  TapGestureRecognizer({ this.arena, this.onTap });

  PointerRouter router;
  GestureArena arena;
  GestureTapCallback onTap;

  TapState state = TapState.ready;
  int primaryPointer;
  sky.Point initialPosition;
  final List<GestureArenaEntry> entries = new List<GestureArenaEntry>();
  int pointerCount = 0;

  void acceptGesture(int pointer) {
    if (pointer == primaryPointer)
      onTap();
  }

  void rejectGesture(int pointer) {
    state = TapState.defunct;
  }

  void resolve(GestureDisposition disposition) {
    for (GestureArenaEntry entry in entries)
      entry.resolve(disposition);
  }

  void _handleEvent(sky.Event e) {
    assert(state != TapState.ready);
    if (e is! sky.PointerEvent)
      return;
    sky.PointerEvent event = e;
    if (state == TapState.possible) {
      if (event.pointer == primaryPointer) {
        if (event.type == 'pointerup') {
          resolve(GestureDisposition.accepted);
        } else if (event.type == 'pointermove') {
          sky.Offset delta = _getPoint(event) - initialPosition;
          if (delta.distance > kTouchSlop)
            resolve(GestureDisposition.rejected);
        }
      }
    }
    if (event.type == 'pointerup' || event.type == 'pointercancel') {
      router.unsubscribe(event.pointer, _handleEvent);
      if (--pointerCount == 0)
        state = TapState.ready;
    }
  }

  void addPointer(sky.PointerEvent event) {
    assert(event.type == 'pointerdown');
    router.subscribe(event.pointer, _handleEvent);
    pointerCount++;
    entries.add(arena.add(event.pointer, this));
    if (state == TapState.ready) {
      primaryPointer = event.pointer;
      state = TapState.possible;
      initialPosition = _getPoint(event);
    }
  }

}
