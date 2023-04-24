// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:js_interop';

import 'package:ui/src/engine.dart';

Future<void> awaitNextFrame() {
  final Completer<void> completer = Completer<void>();
  domWindow.requestAnimationFrame((JSNumber time) => completer.complete());
  return completer.future;
}
