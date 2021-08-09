// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

void main() {}

class SomeClass {
  int i;
  SomeClass(this.i);
}

void giveObjectToNative(Object someObject) native 'GiveObjectToNative';

void signalDone() native 'SignalDone';

@pragma('vm:entry-point')
void callGiveObjectToNative() {
  giveObjectToNative(SomeClass(123));
}

@pragma('vm:entry-point')
void testClearLater() {
  giveObjectToNative(SomeClass(123));
  signalDone();
}

@pragma('vm:entry-point')
@FfiNative<Void Function()>('Nop', isLeaf: true)
external void nop();

@FfiNative<Handle Function(Handle)>('Echo')
external Object echo(Object msg);

@pragma('vm:entry-point')
void callEcho() {
  if (echo('Hello World!') == 'Hello World!') {
    signalDone();
  }
}
