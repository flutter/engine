// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/hit_test.dart';

class GestureEvent {
}

typedef void GestureListener(GestureEvent event);

enum GestureDisposition {
  possible,
  accepted,
  rejected
}

abstract class GestureRecognizer {
  const GestureRecognizer();

  GestureState createGestureState();
  GestureDisposition observeEvent(GestureState state, sky.Event event);
  void notifyListeners(GestureState state, GestureDisposition disposition);
}

class GestureState {
  void addListener(GestureListener listener) {
    _listeners.add(listener);
  }

  void removeListener(GestureListener listener) {
    _listeners.remove(listener);
  }

  void notifyListeners(GestureEvent event) {
    for (GestureListener listener in new List<GestureListener>.from(_listeners))
      listener(event);
  }

  final List<GestureListener> _listeners = new List<GestureListener>();
}

class GestureSubscription {
  GestureSubscription._(this._gestureState, this._listener);
  GestureSubscription._ignored() : _gestureState = null, _listener = null;

  final GestureState _gestureState;
  final GestureListener _listener;

  void cancel() {
    if (_gestureState != null)
      _gestureState.removeListener(_listener);
  }
}

int _getPointer(sky.Event event) {
  if (event is sky.PointerEvent)
    return event.pointer;
  if (event is sky.GestureEvent)
    return event.primaryPointer;
  return null;
}

class PointerState {
  final Map<GestureRecognizer, GestureState> _recognizers = new Map<GestureRecognizer, GestureState>();
  GestureRecognizer _choosenRecognizer;
  GestureState _choosenGestureState;

  GestureSubscription detectGesture(GestureRecognizer recognizer, GestureListener listener) {
    if (_choosenRecognizer == recognizer) {
      assert(_recognizers.isEmpty);
      _choosenGestureState.addListener(listener);
      return new GestureSubscription._(_choosenGestureState, listener);
    }
    if (_choosenRecognizer != null) {
      assert(_recognizers.isEmpty);
      return new GestureSubscription._ignored();
    }
    GestureState gestureState = _recognizers.putIfAbsent(recognizer, () => recognizer.createGestureState());
    assert(gestureState != null);
    gestureState.addListener(listener);
    return new GestureSubscription._(gestureState, listener);
  }

  void _forEachRecognizer(void f(GestureRecognizer r, GestureState s)) {
    new Map<GestureRecognizer, GestureState>.from(_recognizers).forEach(f);
  }

  void _attemptToChooseRecognizer(sky.Event event) {
    assert(_choosenRecognizer == null);
    _forEachRecognizer((GestureRecognizer recognizer, GestureState state) {
      if (_choosenRecognizer != null)
        return;
      GestureDisposition disposition = recognizer.observeEvent(state, event);
      if (disposition == GestureDisposition.accepted) {
        _choosenRecognizer = recognizer;
      } else if (disposition == GestureDisposition.rejected) {
        recognizer.notifyListeners(state, GestureDisposition.rejected);
        _recognizers.remove(recognizer);
      }
    });
    if (_choosenRecognizer == null && _recognizers.length == 1)
      _choosenRecognizer = _recognizers.keys.first;
    if (_choosenRecognizer == null)
      return;
    _choosenGestureState = _recognizers[_choosenRecognizer];
    _forEachRecognizer((GestureRecognizer recognizer, GestureState state) {
      if (recognizer == _choosenRecognizer)
        return;
      recognizer.notifyListeners(state, GestureDisposition.rejected);
    });
    _recognizers.clear();
  }

  void _notifyRecognizers(sky.Event event) {
    if (_choosenRecognizer != null) {
      assert(_recognizers.isEmpty);
      assert(_choosenGestureState != null);
      _choosenRecognizer.notifyListeners(_choosenGestureState, GestureDisposition.accepted);
    } else {
      _forEachRecognizer((GestureRecognizer recognizer, GestureState state) {
        recognizer.notifyListeners(state, GestureDisposition.possible);
      });
    }
  }

  void handleEvent(sky.Event event) {
    if (_choosenRecognizer == null) {
      _attemptToChooseRecognizer(event);
    } else {
      _choosenRecognizer.observeEvent(_choosenGestureState, event);
    }
    _notifyRecognizers(event);
  }
}

class GestureManager extends HitTestTarget {
  final Map<int, PointerState> _pointers = new Map<int, PointerState>();

  GestureSubscription detectGesture(int pointer, GestureRecognizer recognizer, GestureListener listener) {
    assert(pointer != null);
    assert(recognizer != null);
    assert(listener != null);
    PointerState pointerState = _pointers.putIfAbsent(pointer, () => new PointerState());
    return pointerState.detectGesture(recognizer, listener);
  }

  EventDisposition handleEvent(sky.Event event, HitTestEntry entry) {
    int pointer = _getPointer(event);
    if (pointer == null)
      return EventDisposition.ignored;
    PointerState pointerState = _pointers[pointer];
    if (pointerState == null)
      return EventDisposition.ignored;
    pointerState.handleEvent(event);
    // TODO(abarth): When should we remove pointerState? It's tempting to remove
    // it when we see the pointerup, but we still might get gesture events from
    // the platform recognizer that we need to process. We probably need some
    // event that marks the end of the gesture packet.
    return EventDisposition.processed;
  }

}
