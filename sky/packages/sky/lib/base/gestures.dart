// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/gesture_manager.dart';

typedef void GestureTapCallback();

abstract class TapGestureRecognizer extends GestureRecognizer {
  TapGestureRecognizer(GestureManager manager, { this.onTap }) : super(manager);

  GestureTapCallback onTap;
  final List<GestureSubscription> _subscriptions = new List<GestureSubscription>();

  void handlePointerDown(sky.PointerEvent event) {
    _subscriptions.add(manager.addGestureRecognizer(event.pointer, this));
  }

  void dispose() {
    for (GestureSubscription subscription in _subscriptions)
      subscription.cancel();
    _subscriptions.clear();
  }

  GestureDisposition observeEvent(sky.Event event) {
    if (event.type == 'gesturetap')
      return GestureDisposition.accepted;
    return GestureDisposition.possible;
  }

  void notifyListeners(GestureDisposition disposition) {
    if (disposition == GestureDisposition.accepted)
      onTap();
  }
}
















abstract class _GestureEventRecognizer extends GestureRecognizer {
  _GestureEventRecognizer(GestureManager manager) : super(manager);

  sky.GestureEvent _bufferedEvents;

  void notifyListeners(GestureDisposition disposition) {
    if (disposition == GestureDisposition.rejected)
      return;
    for (sky.GestureEvent event in _bufferedEvents)
      state.notifyListeners(_convertEvent(event));
    state._bufferedEvents.clear();
  }

  GestureEvent _convertEvent(sky.GestureEvent event);
}

abstract class _TypeBasedGestureEventRecognizer extends _GestureEventRecognizer {
  const _TypeBasedGestureEventRecognizer();

  GestureDisposition observeEvent(_GestureEventState state, sky.Event event) {
    if (_acceptType(event.type)) {
      state._bufferedEvents.add(event);
      return GestureDisposition.accepted;
    }
    return GestureDisposition.possible;
  }

  bool _acceptType(String type);
}

class _TapRecognizer extends _TypeBasedGestureEventRecognizer {
  const _TapRecognizer();

  bool _acceptType(String type) => type == 'gesturetap';

  GestureEvent _convertEvent(sky.GestureEvent event) {
    return new GestureTap(
      location: new sky.Point(event.x, event.y)
    );
  }
}

class _FlingRecognizer extends _TypeBasedGestureEventRecognizer {
  const _FlingRecognizer();

  bool _acceptType(String type) => type == 'gestureflingstart' || type == 'gestureflingcancel';

  GestureEvent _convertEvent(sky.GestureEvent event) {
    if (event.type == 'gestureflingstart')
      return new GestureFlingStart(velocity: new sky.Offset(event.velocityX, event.velocityY));
    assert(event.type == 'gestureflingcancel');
    return new GestureFlingCancel();
  }
}

class _ScrollRecognizer extends _GestureEventRecognizer {
  const _ScrollRecognizer();

  GestureDisposition observeEvent(_GestureEventState state, sky.Event event) {
    if (event.type == 'gestureflingstart' || event.type == 'gestureflingcancel') {
      state._bufferedEvents.add(event);
      if (_matchesScrollDirection(event))
        return GestureDisposition.accepted;
    }
    return GestureDisposition.possible;
  }

  bool _matchesScrollDirection(sky.GestureEvent event) => true;

  GestureEvent _convertEvent(sky.GestureEvent event) {
    if (event.type == 'gesturescrollstart')
      return new GestureScrollStart(scroll: new sky.Offset(event.dx, event.dy));
    assert(event.type == 'gesturescrollupdate');
    return new GestureScrollUpdate(scroll: new sky.Offset(event.dx, event.dy));
  }
}

class _VerticalScrollRecognizer extends _ScrollRecognizer {
  const _VerticalScrollRecognizer();

  bool _matchesScrollDirection(sky.GestureEvent event) => event.dy > event.dx;
}

class _HorizontalScrollRecognizer extends _ScrollRecognizer {
  const _HorizontalScrollRecognizer();

  bool _matchesScrollDirection(sky.GestureEvent event) => event.dy < event.dx;
}

class GestureTap extends GestureEvent {
  GestureTap({ this.location });

  final sky.Point location;
}

class GestureFlingStart extends GestureEvent {
  GestureFlingStart({ this.velocity });

  final sky.Offset velocity;
}

class GestureFlingCancel extends GestureEvent {
  GestureFlingCancel();
}

class GestureScroll extends GestureEvent {
  GestureScroll({ this.scroll });

  final sky.Offset scroll;
}

class GestureScrollStart extends GestureScroll {
  GestureScrollStart({ sky.Offset scroll }) : super(scroll: scroll);
}

class GestureScrollUpdate extends GestureScroll {
  GestureScrollUpdate({ sky.Offset scroll }) : super(scroll: scroll);
}

const tapGesture = const _TapRecognizer();
const flingGesture = const _FlingRecognizer();
const verticalScrollGesture = const _VerticalScrollRecognizer();
const horizontalScrollGesture = const _HorizontalScrollRecognizer();
