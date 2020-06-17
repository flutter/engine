// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:async';
import 'dart:html' as html;
import 'dart:js_util' as js_util;
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

const MethodCodec codec = JSONMethodCodec();

void emptyCallback(ByteData data) {}

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  setUp(() async {
    await window.debugSwitchBrowserHistory(useSingle: true);
  });

  tearDown(() async {
    await window.debugResetHistory();
  });

  test('window.defaultRouteName should not change', () async {
    final TestUrlStrategy strategy = TestUrlStrategy.fromEntry(
      TestHistoryEntry('initial state', null, '/initial'),
    );
    await window.debugConvertAndSetUrlStrategy(strategy);
    expect(window.defaultRouteName, '/initial');

    // Changing the URL in the address bar later shouldn't affect [window.defaultRouteName].
    strategy.replaceState(null, null, '/newpath');
    expect(window.defaultRouteName, '/initial');
  });

  test('window.defaultRouteName should reset after navigation platform message',
      () async {
    await window.debugConvertAndSetUrlStrategy(TestUrlStrategy.fromEntry(
      TestHistoryEntry('initial state', null, '/initial'),
    ));
    // Reading it multiple times should return the same value.
    expect(window.defaultRouteName, '/initial');
    expect(window.defaultRouteName, '/initial');
    window.sendPlatformMessage(
      'flutter/navigation',
      JSONMethodCodec().encodeMethodCall(MethodCall(
        'routeUpdated',
        <String, dynamic>{'routeName': '/bar'},
      )),
      emptyCallback,
    );
    // After a navigation platform message, [window.defaultRouteName] should
    // reset to "/".
    expect(window.defaultRouteName, '/');
  });

  test('can disable location strategy', () async {
    final testStrategy = TestUrlStrategy.fromEntry(
      TestHistoryEntry('initial state', null, '/'),
    );
    await window.debugConvertAndSetUrlStrategy(testStrategy);

    expect(window.urlStrategy, isNotNull);
    // A single listener should've been setup.
    expect(testStrategy.listeners, hasLength(1));
    // The initial entry should be there, plus another "flutter" entry.
    expect(testStrategy.history, hasLength(2));
    expect(testStrategy.history[0].state,
        <String, dynamic>{'origin': true, 'state': 'initial state'});
    expect(testStrategy.history[1].state, <String, bool>{'flutter': true});
    expect(testStrategy.currentEntry, testStrategy.history[1]);

    // Now, let's disable location strategy and make sure things get cleaned up.
    expect(() => jsSetUrlStrategy(null), returnsNormally);
    // The url strategy is teared down asynchronously.
    await Future<void>.delayed(Duration.zero);
    expect(window.urlStrategy, isNull);

    // The listener is removed asynchronously.
    await Future<void>.delayed(const Duration(milliseconds: 10));

    // No more listeners.
    expect(testStrategy.listeners, isEmpty);
    // History should've moved back to the initial state.
    expect(testStrategy.history[0].state, "initial state");
    expect(testStrategy.currentEntry, testStrategy.history[0]);
  });

  test('js interop throws on wrong type', () {
    expect(() => jsSetUrlStrategy(123), throwsA(anything));
    expect(() => jsSetUrlStrategy('foo'), throwsA(anything));
    expect(() => jsSetUrlStrategy(false), throwsA(anything));
  });
}

void jsSetUrlStrategy(dynamic strategy) {
  js_util.callMethod(
    html.window,
    '_flutter_web_set_location_strategy',
    <dynamic>[strategy],
  );
}
