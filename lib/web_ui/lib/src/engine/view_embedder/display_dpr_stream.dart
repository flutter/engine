// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:js_interop';

import 'package:ui/src/engine/display.dart';
import 'package:ui/src/engine/dom.dart';
import 'package:ui/ui.dart' as ui show Display;

/// Determines if high contrast is enabled using media query 'forced-colors: active' for Windows
class DisplayDprStream {
  DisplayDprStream(ui.Display display) : _display = display, _currentDpr = display.devicePixelRatio {
    // Start listening to DPR changes.
    _subscribeToMediaQuery();
  }

  /// A singleton instance of DisplayDprObserver.
  static DisplayDprStream instance = DisplayDprStream(EngineFlutterDisplay.instance);

  // The display object that will provide the DPR information.
  final ui.Display _display;

  // Last reported value of DPR.
  double _currentDpr;

  // Controls the [dprChanged] Stream.
  final StreamController<double> _dprStreamController = StreamController<double>.broadcast();

  // Listens for the `_currentDpr` to change.
  late DomMediaQueryList _dprMediaQuery;

  // Creates the media query for the latest known DPR value, and adds a change listener to it.
  void _subscribeToMediaQuery() {
    _dprMediaQuery = domWindow.matchMedia('(resolution: ${_currentDpr}dppx)');
    _dprMediaQuery.addEventListenerWithOptions(
      'change',
      createDomEventListener(_onDprMediaQueryChange),
      <String, Object> {
        // We only listen `once` because this event only triggers once when the
        // DPR changes from `_currentDpr`. Once that happens, we need a new
        // `_dprMediaQuery` that is watching the new `_currentDpr`.
        //
        // By using `once`, we don't need to worry about detaching the event
        // listener from the old mediaQuery after we're done with it.
        'once': true,
        'passive': true,
      });
  }

  // Handler of the _dprMediaQuery 'change' event.
  //
  // This calls subscribe again because events are listened to with `once: true`.
  JSVoid _onDprMediaQueryChange(DomEvent _) {
    _currentDpr = _display.devicePixelRatio;
    _dprStreamController.add(_currentDpr);
    // Re-subscribe...
    _subscribeToMediaQuery();
  }

  /// A stream that emits the latest value of [EngineFlutterDisplay.instance.devicePixelRatio].
  Stream<double> get dprChanged => _dprStreamController.stream;
}
