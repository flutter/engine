// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:fuchsia_scenic_flutter/fuchsia_view.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('FuchsiaViewsService register and deregister', () async {
    final fuchsiaViewsService = FuchsiaViewsService.instance;
    expect(fuchsiaViewsService, isNotNull);

    // Should receive callback for registered view.
    MethodCall? callback;
    fuchsiaViewsService.register(42, (call) async {
      callback = call;
    });
    await fuchsiaViewsService.platformViewChannel.binaryMessenger
        .handlePlatformMessage(
            'flutter/platform_views',
            JSONMethodCodec().encodeMethodCall(MethodCall(
                'View.viewConnected', <String, dynamic>{'viewId': 42})),
            null);
    expect(callback?.method, 'View.viewConnected');

    // Should receive callback for missing viewId.
    callback = null;
    await fuchsiaViewsService.platformViewChannel.binaryMessenger
        .handlePlatformMessage(
            'flutter/platform_views',
            JSONMethodCodec()
                .encodeMethodCall(MethodCall('View.viewDisconnected')),
            null);
    expect(callback?.method, 'View.viewDisconnected');

    // Should NOT receive callback for unknown viewId.
    callback = null;
    await fuchsiaViewsService.platformViewChannel.binaryMessenger
        .handlePlatformMessage(
            'flutter/platform_views',
            JSONMethodCodec().encodeMethodCall(MethodCall(
                'View.viewDisconnected', <String, dynamic>{'viewId': 0xbad})),
            null);
    expect(callback, isNull);

    // Should NOT receive callback after deregistering.
    callback = null;
    fuchsiaViewsService.deregister(42);
    await fuchsiaViewsService.platformViewChannel.binaryMessenger
        .handlePlatformMessage(
            'flutter/platform_views',
            JSONMethodCodec().encodeMethodCall(MethodCall(
                'View.viewDisconnected', <String, dynamic>{'viewId': 42})),
            null);
    expect(callback, isNull);
  });

  test('FuchsiaViewsService create/update/destroy/requestFocus', () async {
    final fuchsiaViewsService = FuchsiaViewsService.instance;
    expect(fuchsiaViewsService, isNotNull);

    // Capture calls made on platformViewChannel.
    late MethodCall invokedMethod;
    Future<dynamic> result = Future.value(0);
    fuchsiaViewsService.platformViewChannel
        .setMockMethodCallHandler((call) async {
      invokedMethod = call;
      return result;
    });

    await fuchsiaViewsService.createView(42);
    expect(invokedMethod.method, 'View.create');
    expect(invokedMethod.arguments, <String, dynamic>{
      'viewId': 42,
      'hitTestable': true,
      'focusable': true,
      'viewOcclusionHintLTRB': <double>[0, 0, 0, 0],
    });

    await fuchsiaViewsService.updateView(
      42,
      hitTestable: false,
      focusable: false,
      viewOcclusionHint: Rect.fromLTRB(10, 10, 20, 30),
    );
    expect(invokedMethod.method, 'View.update');
    expect(invokedMethod.arguments, <String, dynamic>{
      'viewId': 42,
      'hitTestable': false,
      'focusable': false,
      'viewOcclusionHintLTRB': <double>[10, 10, 20, 30],
    });

    await fuchsiaViewsService.destroyView(42);
    expect(invokedMethod.method, 'View.dispose');
    expect(invokedMethod.arguments, <String, dynamic>{'viewId': 42});

    await fuchsiaViewsService.requestFocus(42);
    expect(invokedMethod.method, 'View.requestFocus');
    expect(invokedMethod.arguments, <String, dynamic>{'viewRef': 42});
  });
}
