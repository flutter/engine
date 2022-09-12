// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:isolate';
import 'dart:ui';

import 'package:litetest/litetest.dart';

typedef StringFunction = String Function();
typedef IntFunction = int Function();

String top() => 'top';

class Foo {
  const Foo();
  static int getInt() => 1;
  double getDouble() => 1.0;
}

const Foo foo = Foo();

@pragma('vm:entry-point')
Future<void> included() async {

}

Future<void> excluded() async {

}

@pragma('vm:entry-point')
Future<void> runCallback(IsolateParam param) async {
  try {
    final Future<dynamic> Function() func = PluginUtilities.getCallbackFromHandle(CallbackHandle.fromRawHandle(param.rawHandle))! as Future<dynamic> Function();
    await func.call();
    param.sendPort.send(true);
  }
  catch (e) {
    print(e);
    param.sendPort.send(false);
  }
}

class IsolateParam {
  const IsolateParam(this.sendPort, this.rawHandle);
  final SendPort sendPort;
  final int rawHandle;
}

void main() {
  test('PluginUtilities Callback Handles', () {
    // Top level callback.
    final CallbackHandle hTop = PluginUtilities.getCallbackHandle(top)!;
    expect(hTop, notEquals(0));
    expect(PluginUtilities.getCallbackHandle(top), hTop);
    final StringFunction topClosure = PluginUtilities.getCallbackFromHandle(hTop)! as StringFunction;
    expect(topClosure(), 'top');

    // Static method callback.
    final CallbackHandle hGetInt = PluginUtilities.getCallbackHandle(Foo.getInt)!;
    expect(hGetInt, notEquals(0));
    expect(PluginUtilities.getCallbackHandle(Foo.getInt), hGetInt);
    final IntFunction getIntClosure = PluginUtilities.getCallbackFromHandle(hGetInt)! as IntFunction;
    expect(getIntClosure(), 1);

    // Instance method callbacks cannot be looked up.
    const Foo foo = Foo();
    expect(PluginUtilities.getCallbackHandle(foo.getDouble), isNull);

    // Anonymous closures cannot be looked up.
    final Function anon = (int a, int b) => a + b; // ignore: prefer_function_declarations_over_variables
    expect(PluginUtilities.getCallbackHandle(anon), isNull);
  });

  test('PluginUtilities Callback Handles in Isolate', () async {
    if (!const bool.fromEnvironment('dart.vm.product')) {
      return;
    }
    ReceivePort port = ReceivePort();
    await Isolate.spawn(runCallback, IsolateParam(port.sendPort, PluginUtilities.getCallbackHandle(included)!.toRawHandle()));
    expect(await port.first, true);
    port.close();
    port = ReceivePort();
    await Isolate.spawn(runCallback, IsolateParam(port.sendPort, PluginUtilities.getCallbackHandle(excluded)!.toRawHandle()));
    expect(await port.first, false);
    port.close();
  });
}
