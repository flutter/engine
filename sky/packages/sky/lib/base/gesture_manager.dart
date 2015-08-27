// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/hit_test.dart';

enum GestureDisposition {
  possible,
  accepted,
  rejected
}

abstract class GestureRecognizer {
  GestureRecognizer(this.manager);

  final GestureManager manager;

  void handlePointerDown(sky.PointerEvent event);
  void dispose();

  GestureDisposition observeEvent(sky.Event event);
  void notifyListeners(GestureDisposition disposition);
}

int _getPointer(sky.Event event) {
  if (event is sky.PointerEvent)
    return event.pointer;
  if (event is sky.GestureEvent)
    return event.primaryPointer;
  return null;
}

class GestureSubscription {
  PointerState _state;
  GestureRecognizer _recognizer;

  GestureSubscription._(this._state, this._recognizer);

  void cancel() {
    if (_state != null)
      _state._recognizers.remove(_recognizer);
  }
}

class PointerState {
  final List<GestureRecognizer> _recognizers = new List<GestureRecognizer>();
  GestureRecognizer _choosenRecognizer;

  GestureSubscription detectGesture(GestureRecognizer recognizer) {
    if (_choosenRecognizer != null) {
      assert(_recognizers.isEmpty);
      return new GestureSubscription._(null, null);
    }
    assert(!_recognizers.contains(recognizer));
    _recognizers.add(recognizer);
    return new GestureSubscription._(this, recognizer);
  }

  GestureRecognizer _attemptToChooseRecognizer(sky.Event event) {
    assert(_choosenRecognizer == null);
    for (GestureRecognizer recognizer in new List<GestureRecognizer>.from(_recognizers)) {
      GestureDisposition disposition = recognizer.observeEvent(event);
      if (disposition == GestureDisposition.accepted) {
        return recognizer;
      } else if (disposition == GestureDisposition.rejected) {
        recognizer.notifyListeners(GestureDisposition.rejected);
        _recognizers.remove(recognizer);
      }
    }
    assert(_choosenRecognizer == null);
    if (_recognizers.length == 1)
      return _recognizers.first;
    return null;
  }

  void _rejectRemainingRecognizers() {
    for (GestureRecognizer recognizer in _recognizers) {
      if (recognizer != _choosenRecognizer)
        recognizer.notifyListeners(GestureDisposition.rejected);
    }
    _recognizers.clear();
  }

  void _notifyRecognizers(sky.Event event) {
    if (_choosenRecognizer != null) {
      assert(_recognizers.isEmpty);
      _choosenRecognizer.notifyListeners(GestureDisposition.accepted);
    } else {
      for (GestureRecognizer recognizer in _recognizers) {
        recognizer.notifyListeners(GestureDisposition.possible);
      }
    }
  }

  void handleEvent(sky.Event event) {
    if (_choosenRecognizer == null) {
      _choosenRecognizer = _attemptToChooseRecognizer(event);
      if (_choosenRecognizer != null)
        _rejectRemainingRecognizers();
    } else {
      _choosenRecognizer.observeEvent(event);
    }
    _notifyRecognizers(event);
  }
}

class GestureManager extends HitTestTarget {
  final Map<int, PointerState> _pointers = new Map<int, PointerState>();

  GestureSubscription addGestureRecognizer(int pointer, GestureRecognizer recognizer) {
    assert(pointer != null);
    assert(recognizer != null);
    PointerState pointerState = _pointers.putIfAbsent(pointer, () => new PointerState());
    return pointerState.detectGesture(recognizer);
  }

  EventDisposition handleEvent(sky.Event event, HitTestEntry entry) {
    int pointer = _getPointer(event);
    if (pointer == null)
      return EventDisposition.ignored;
    PointerState pointerState = _pointers[pointer];
    if (pointerState == null)
      return EventDisposition.ignored;
    pointerState.handleEvent(event);
    return EventDisposition.processed;
  }

}
