// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:isolate';
import 'dart:ui';

import 'package:ffi/ffi.dart';
import 'package:litetest/litetest.dart';

import 'spawn_helper.dart';

@Native<Handle Function(Pointer<Utf8>)>(symbol: 'LoadLibraryFromKernel')
external Object _loadLibraryFromKernel(Pointer<Utf8> path);

@Native<Handle Function(Pointer<Utf8>, Pointer<Utf8>)>(symbol: 'LookupEntryPoint')
external Object _lookupEntryPoint(Pointer<Utf8> library, Pointer<Utf8> name);

@Native<Void Function(Pointer<Utf8>, Pointer<Utf8>)>(symbol: 'Spawn')
external void _spawn(Pointer<Utf8> entrypoint, Pointer<Utf8> route);

void spawn(
    {required SendPort port, String entrypoint = 'main', String route = '/'}) {
  // Get off the UI isolate, otherwise there will be a current isolate that
  // cannot safely be exited.
  Isolate.run(() {
    assert(
      entrypoint != 'main' || route != '/',
      'Spawn should not be used to spawn main with the default route name',
    );
    IsolateNameServer.registerPortWithName(port, route);
    _spawn(entrypoint.toNativeUtf8(), route.toNativeUtf8());
  });
}

const String kTestEntrypointRouteName = 'testEntrypoint';

@pragma('vm:entry-point')
void testEntrypoint() {
  expect(PlatformDispatcher.instance.defaultRouteName, kTestEntrypointRouteName);
  IsolateNameServer.lookupPortByName(kTestEntrypointRouteName)!.send(null);
}

void main() {
  test('Spawn a different entrypoint with a special route name', () async {
    final ReceivePort port = ReceivePort();
    spawn(port: port.sendPort, entrypoint: 'testEntrypoint', route: kTestEntrypointRouteName);
    expect((await port.first), isNull);
    port.close();
  });

  test('Lookup entrypoint and execute', () {
    final String libraryPath = 'file://${const String.fromEnvironment('kFlutterSrcDirectory')}/testing/dart/spawn_helper.dart';
    expect(
      (_lookupEntryPoint(
        libraryPath.toNativeUtf8(),
        'echoInt'.toNativeUtf8(),
      ) as int Function(int))(42),
      42,
    );
  });

  test('Load from kernel', () {
    final String kernelPath = '${const String.fromEnvironment('kFlutterBuildDirectory')}/spawn_helper.dart.dill';
    expect(
      _loadLibraryFromKernel(kernelPath.toNativeUtf8()) is void Function(),
      true,
    );

    expect(_loadLibraryFromKernel('fake-path'.toNativeUtf8()), null);
  });
}
