// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:sky' as sky;

import 'package:sky/base/gesture_manager.dart';
import 'package:sky/base/gestures.dart';
import 'package:sky/rendering/sky_binding.dart';
import 'package:sky/widgets/basic.dart';

Function _filter(bool predicate(GestureEvent event), GestureListener listener) {
  return (GestureEvent event) {
    if (predicate(event))
      listener(event);
  };
}

class Gesture extends StatefulComponent {

  Gesture({
    Key key,
    this.child,
    this.onTap,
    this.onFlingStart,
    this.onFlingCancel,
    this.onVerticalScroll,
    this.onHorizontalScroll
  }) : super(key: key);

  Widget child;
  GestureListener onTap;
  GestureListener onFlingStart;
  GestureListener onFlingCancel;
  GestureListener onVerticalScroll;
  GestureListener onHorizontalScroll;

  final List<GestureSubscription> _subscriptions = new List<GestureSubscription>();

  void syncConstructorArguments(Gesture source) {
    child = source.child;
    onTap = source.onTap;
    onFlingStart = source.onFlingStart;
    onFlingCancel = source.onFlingCancel;
    onVerticalScroll = source.onVerticalScroll;
    onHorizontalScroll = source.onHorizontalScroll;
    super.syncConstructorArguments(source);
  }

  void didUnmount() {
    super.didUnmount();
    for (GestureSubscription subscription in _subscriptions)
      subscription.cancel();
    _subscriptions.clear();
  }

  void _detectGesture(sky.PointerEvent event, GestureRecognizer recognizer, GestureListener listener) {
    GestureManager gestureManager = SkyBinding.instance.gestureManager;
    _subscriptions.add(gestureManager.detectGesture(event.pointer, recognizer, listener));
  }

  EventDisposition _handlePointerDown(sky.PointerEvent event) {
    if (onTap != null)
      _detectGesture(event, tapGesture, onTap);
    if (onFlingStart != null)
      _detectGesture(event, flingGesture, _filter((e) => e is GestureFlingStart, onFlingStart));
    if (onFlingCancel != null)
      _detectGesture(event, flingGesture, _filter((e) => e is GestureFlingCancel, onFlingCancel));
    if (onVerticalScroll != null)
      _detectGesture(event, verticalScrollGesture, onVerticalScroll);
    if (onHorizontalScroll != null)
      _detectGesture(event, verticalScrollGesture, onHorizontalScroll);
    return EventDisposition.processed;
  }

  Widget build() {
    return new Listener(
      onPointerDown: _handlePointerDown,
      child: child
    );
  }

}
