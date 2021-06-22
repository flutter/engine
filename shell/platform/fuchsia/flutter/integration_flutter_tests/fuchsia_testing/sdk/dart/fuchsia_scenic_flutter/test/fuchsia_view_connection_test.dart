// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_as, dead_code, null_check_always_fails

import 'dart:ui';

import 'package:fidl_fuchsia_ui_views/fidl_async.dart';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:fuchsia_scenic_flutter/fuchsia_view.dart';
import 'package:mockito/mockito.dart';
import 'package:zircon/zircon.dart';

// ignore: implementation_imports
import 'package:fuchsia_scenic_flutter/src/pointer_injector.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  test('FuchsiaViewConnection', () async {
    bool? connectedCalled;
    bool? disconnectedCalled;
    final connection = TestFuchsiaViewConnection(
      _mockViewHolderToken(),
      onViewConnected: (_) => connectedCalled = true,
      onViewDisconnected: (_) => disconnectedCalled = true,
    );

    await connection.connect();
    verify(connection.fuchsiaViewsService.createView(
      42,
      hitTestable: true,
      focusable: true,
      viewOcclusionHint: Rect.zero,
    ));

    final methodCallback =
        verify(connection.fuchsiaViewsService.register(42, captureAny))
            .captured
            .single;
    expect(methodCallback, isNotNull);

    methodCallback(MethodCall('View.viewConnected'));
    methodCallback(MethodCall('View.viewDisconnected'));

    expect(connectedCalled, isTrue);
    expect(disconnectedCalled, isTrue);
  });

// TODO(fxb/69607): Re-enable this test. Rework to account for nullability.
//   test('FuchsiaViewConnection.usePointerInjection', () async {
//     bool? connectedCalled;
//     bool? disconnectedCalled;
//     final connection = TestFuchsiaViewConnection(
//       _mockViewHolderToken(),
//       viewRef: _mockViewRef(),
//       onViewConnected: (_) => connectedCalled = true,
//       onViewDisconnected: (_) => disconnectedCalled = true,
//       usePointerInjection: true,
//     );

//     final handle = MockHandle();
//     when(handle.duplicate(any)).thenReturn(handle);
//     when(connection.startupContext.viewRef).thenReturn(handle);

//     await connection.connect();
//     verify(connection.platformViewChannel.invokeMethod('View.create', {
//       'viewId': 42,
//       'hitTestable': true,
//       'focusable': true,
//     }));

//     final methodCallback =
//         verify(connection.platformViewChannel.setMethodCallHandler(captureAny))
//             .captured
//             .single;
//     expect(methodCallback, isNotNull);

//     methodCallback(MethodCall('View.viewConnected'));
//     methodCallback(MethodCall('View.viewDisconnected'));

//     expect(connectedCalled, isTrue);
//     expect(disconnectedCalled, isTrue);

//     verify((connection.pointerInjector as MockPointerInjector).register(
//       hostViewRef: anyNamed('hostViewRef'),
//       viewRef: anyNamed('viewRef'),
//       viewport: anyNamed('viewport'),
//     ));
//     verify(connection.pointerInjector.dispose());

//     // Test pointer dispatch works.
//     final addedEvent = PointerAddedEvent();
//     await connection.dispatchPointerEvent(addedEvent);
//     verify((connection.pointerInjector as MockPointerInjector).dispatchEvent(
//         pointer: anyNamed('pointer'), viewport: anyNamed('viewport')));
//   });
}

// ViewRef _mockViewRef() {
//   final handle = MockHandle();
//   when(handle.isValid).thenReturn(true);
//   when(handle.duplicate(any)).thenReturn(handle);
//   final eventPair = MockEventPair();
//   when(eventPair.handle).thenReturn(handle);
//   when(eventPair.isValid).thenReturn(true);
//   final viewRef = MockViewRef();
//   when(viewRef.reference).thenReturn(eventPair);
//   return viewRef;
// }

ViewHolderToken _mockViewHolderToken() {
  final handle = MockHandle();
  when(handle.handle).thenReturn(42);
  final eventPair = MockEventPair();
  when(eventPair.handle).thenReturn(handle);
  when(eventPair.isValid).thenReturn(true);
  final viewHolderToken = MockViewHolderToken();
  when(viewHolderToken.value).thenReturn(eventPair);

  return viewHolderToken;
}

class TestFuchsiaViewConnection extends FuchsiaViewConnection {
  final _fuchsiaViewsService = MockFuchsiaViewsService();
  final _pointerInjector = MockPointerInjector();

  TestFuchsiaViewConnection(
    ViewHolderToken viewHolderToken, {
    ViewRef? viewRef,
    FuchsiaViewConnectionCallback? onViewConnected,
    FuchsiaViewConnectionCallback? onViewDisconnected,
    FuchsiaViewConnectionStateCallback? onViewStateChanged,
    bool usePointerInjection = false,
  }) : super(
          viewHolderToken,
          viewRef: viewRef,
          onViewConnected: onViewConnected,
          onViewDisconnected: onViewDisconnected,
          onViewStateChanged: onViewStateChanged,
          usePointerInjection: usePointerInjection,
        );

  @override
  FuchsiaViewsService get fuchsiaViewsService => _fuchsiaViewsService;

  @override
  PointerInjector get pointerInjector => _pointerInjector;
}

class MockFuchsiaViewsService extends Mock implements FuchsiaViewsService {}

class MockViewHolderToken extends Mock implements ViewHolderToken {
  @override
  int get hashCode => super.noSuchMethod(Invocation.method(#hashCode, []));

  @override
  bool operator ==(dynamic other) =>
      super.noSuchMethod(Invocation.method(#==, [other]));
}

class MockViewRef extends Mock implements ViewRef {
  @override
  int get hashCode => super.noSuchMethod(Invocation.method(#hashCode, []));

  @override
  bool operator ==(dynamic other) =>
      super.noSuchMethod(Invocation.method(#==, [other]));
}

class MockEventPair extends Mock implements EventPair {}

class MockHandle extends Mock implements Handle {}

class MockPointerInjector extends Mock implements PointerInjector {}
