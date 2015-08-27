// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/pointer_router.dart';
import 'package:sky/gestures/arena.dart';

sky.Offset _getOffset(sky.PointerEvent event) {
  return new sky.Offset(event.dx, event.dy);
}

enum ScrollState {
  ready,
  possible,
  accepted
}

typedef void GestureScrollCallback(sky.Offset scrollDelta);

class ScrollGestureRecognizer extends GestureArenaMember {
  ScrollGestureRecognizer({ this.arena, this.onScrollStart, this.onScrollUpdate });

  PointerRouter router;
  GestureArena arena;
  GestureScrollCallback onScrollStart;
  GestureScrollCallback onScrollUpdate;

  ScrollState state = ScrollState.ready;
  sky.Offset initialOffset;
  final List<GestureArenaEntry> entries = new List<GestureArenaEntry>();
  int primaryPointer;
  int pointerCount = 0;

  void acceptGesture(int pointer) {
    state = ScrollState.accepted;
    onScrollStart(initialOffset);
  }

  void rejectGesture(int pointer) {
  }

  void resolve(GestureDisposition disposition) {
    for (GestureArenaEntry entry in entries)
      entry.resolve(disposition);
  }

  void _handleEvent(sky.Event e) {
    assert(state != ScrollState.ready);
    if (e is! sky.PointerEvent)
      return;
    sky.PointerEvent event = e;
    if (event.type == 'pointermove') {
      if (event.pointer == primaryPointer) {
        sky.Offset offset = _getOffset(event);
        if (state == ScrollState.accepted)
          onScrollUpdate(offset);
        else
          initialOffset += offset;
      }
    } else if (event.type == 'pointerup' || event.type == 'pointercancel') {
      router.unsubscribe(event.pointer, _handleEvent);
      if (--pointerCount == 0)
        state = ScrollState.ready;
    }
  }

  void addPointer(sky.PointerEvent event) {
    assert(event.type == 'pointerdown');
    router.subscribe(event.pointer, _handleEvent);
    pointerCount++;
    entries.add(arena.add(event.pointer, this));

    if (state == ScrollState.ready) {
      state = ScrollState.possible;
      initialOffset = sky.Offset.zero;
    }

    if (state == ScrollState.possible)
      primaryPointer = event.pointer;
  }

}
