// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@TestOn('browser')

import 'dart:js_util';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {

  int callOrder = 1;
  int initCalled = 0;
  int runCalled = 0;
  int cleanCalled = 0;

  setUp(() {
    callOrder = 1;
    initCalled = 0;
    runCalled = 0;
    cleanCalled = 0;
  });

  Future<void> mockInit () async {
    initCalled = callOrder++;
    await Future<void>.delayed(const Duration(milliseconds: 1));
  }

  void mockRunApp () {
    runCalled = callOrder++;
  }
  void mockCleanup () {
    cleanCalled = callOrder++;
  }

  test('now() immediately calls init and run', () async {
    final AppBootstrap bootstrap = AppBootstrap(
      initEngine: mockInit,
      runApp: mockRunApp,
      cleanApp: mockCleanup,
    );

    await bootstrap.now();

    expect(initCalled, 1, reason: 'initEngine should be called first.');
    expect(runCalled, 2, reason: 'runApp should be called after init.');
    expect(cleanCalled, 0, reason: 'cleanApp should never be called.');
  });

  test('engineInitializer runApp() does the same as now()', () async {
    final AppBootstrap bootstrap = AppBootstrap(
      initEngine: mockInit,
      runApp: mockRunApp,
      cleanApp: mockCleanup,
    );

    final FlutterEngineInitializer engineInitializer = bootstrap.prepareCustomEngineInitializer();

    expect(engineInitializer, isNotNull);

    await callMethod<dynamic>(engineInitializer, 'runApp', <Object?>[]);

    expect(initCalled, 1, reason: 'initEngine should be called first.');
    expect(runCalled, 2, reason: 'runApp should be called after init.');
    expect(cleanCalled, 0, reason: 'cleanApp should never be called.');
  });

  test('engineInitializer initEngine() calls init and returns an appRunner', () async {
    final AppBootstrap bootstrap = AppBootstrap(
      initEngine: mockInit,
      runApp: mockRunApp,
      cleanApp: mockCleanup,
    );

    final FlutterEngineInitializer engineInitializer = bootstrap.prepareCustomEngineInitializer();

    final Object maybeAppInitializer = await promiseToFuture<Object>(callMethod<Object>(engineInitializer, 'initializeEngine', <Object?>[]));

    expect(maybeAppInitializer, isA<FlutterAppRunner>());
    expect(initCalled, 1, reason: 'initEngine should have been called.');
    expect(runCalled, 0, reason: 'runApp should not have been called.');
    expect(cleanCalled, 0, reason: 'cleanApp should not have been called.');
  });

  test('appRunner runApp() calls run and returns an appCleaner', () async {
    final AppBootstrap bootstrap = AppBootstrap(
      initEngine: mockInit,
      runApp: mockRunApp,
    );

    final FlutterEngineInitializer engineInitializer = bootstrap.prepareCustomEngineInitializer();

    final Object appInitializer = await promiseToFuture<Object>(callMethod<Object>(engineInitializer, 'initializeEngine', <Object?>[]));
    final Object maybeAppCleaner = await promiseToFuture<Object>(callMethod<Object>(appInitializer, 'runApp', <Object?>[]));

    expect(maybeAppCleaner, isA<FlutterAppCleaner>());
    expect(initCalled, 1, reason: 'initEngine should have been called.');
    expect(runCalled, 2, reason: 'runApp should have been called.');
    expect(cleanCalled, 0, reason: 'cleanApp should not have been called.');
  });

  test('appCleaner cleanApp() calls clean and returns true', () async {
    final AppBootstrap bootstrap = AppBootstrap(
      initEngine: mockInit,
      runApp: mockRunApp,
      cleanApp: mockCleanup,
    );

    final FlutterEngineInitializer engineInitializer = bootstrap.prepareCustomEngineInitializer();

    final Object appInitializer = await promiseToFuture<Object>(callMethod<Object>(engineInitializer, 'initializeEngine', <Object?>[]));
    final Object appCleaner = await promiseToFuture<Object>(callMethod<Object>(appInitializer, 'runApp', <Object?>[]));
    final bool result = await promiseToFuture<bool>(callMethod<Object>(appCleaner, 'cleanApp', <Object?>[]));

    expect(result, isTrue);
    expect(initCalled, 1, reason: 'initEngine should have been called.');
    expect(runCalled, 2, reason: 'runApp should have been called.');
    expect(cleanCalled, 3, reason: 'cleanApp should have been called.');
  });
}
