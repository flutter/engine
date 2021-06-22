// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_as, unnecessary_null_comparison
// ignore_for_file: avoid_catches_without_on_clauses

import 'dart:typed_data';
import 'dart:ui';

import 'package:fidl_fuchsia_ui_pointerinjector/fidl_async.dart';
import 'package:fidl_fuchsia_ui_views/fidl_async.dart';
import 'package:flutter/gestures.dart';
import 'package:fuchsia_logger/logger.dart';
import 'package:fuchsia_services/services.dart';
import 'package:meta/meta.dart';

/// Defines a class that uses the pointer injector service to inject pointer
/// events into child views.
///
/// Requires following services in the environment:
///    fuchsia.ui.pointerinjector.Registry
class PointerInjector {
  final Registry _registry;
  final DeviceProxy _device;

  /// Returns [true] if the PointerInjector is successfully registered.
  bool registered = false;

  /// Constructor used for injecting mocks during testing.
  @visibleForTesting
  PointerInjector(Registry registry, DeviceProxy device)
      : _registry = registry,
        _device = device;

  /// Construct PointerInjector from [/svc].
  factory PointerInjector.fromSvcPath() {
    final registry = RegistryProxy();
    final device = DeviceProxy();
    Incoming.fromSvcPath().connectToService(registry);
    return PointerInjector(registry, device);
  }

  /// Closes connections to services.
  void dispose() {
    if (_registry is RegistryProxy) {
      RegistryProxy proxy = _registry as RegistryProxy;
      proxy.ctrl.close();
    }
    _device.ctrl.close();
  }

  /// Registers with the pointer injector service.
  Future<void> register({
    required ViewRef hostViewRef,
    required ViewRef viewRef,
    required Rect viewport,
  }) async {
    final config = Config(
      deviceId: 1,
      deviceType: DeviceType.touch,
      context: Context.withView(hostViewRef),
      target: Target.withView(viewRef),
      viewport: Viewport(
        extents: _extentFromRect(viewport),
        viewportToContextTransform: _transformFromRect(viewport),
      ),
      dispatchPolicy: DispatchPolicy.exclusiveTarget,
    );

    try {
      await _registry.register(config, _device.ctrl.request());
      registered = true;
    } catch (e) {
      log.warning('Failed to register pointer injector: $e');
    }
  }

  /// Dispatch [PointerEvent] and [Rect] viewport event to embedded child.
  Future<void> dispatchEvent({
    required PointerEvent pointer,
    required Rect? viewport,
  }) async {
    assert(viewport != null && pointer != null);
    final events = <Event>[];

    if (viewport != null) {
      final timestamp = DateTime.now().microsecondsSinceEpoch * 1000;
      final injectorEvent = Event(
        timestamp: timestamp,
        data: Data.withViewport(
          Viewport(
            extents: _extentFromRect(viewport),
            viewportToContextTransform: _transformFromRect(viewport),
          ),
        ),
        traceFlowId: timestamp,
      );
      events.add(injectorEvent);
    }

    if (_isValidPointerEvent(pointer)) {
      final x = pointer.localPosition.dx;
      final y = pointer.localPosition.dy;
      final phase = pointer is PointerDownEvent
          ? EventPhase.add
          : pointer is PointerUpEvent
              ? EventPhase.remove
              : pointer is PointerMoveEvent
                  ? EventPhase.change
                  : EventPhase.cancel;

      final sample = PointerSample(
        pointerId: pointer.device,
        phase: phase,
        positionInViewport: Float32List.fromList([x, y]),
      );
      final injectorEvent = Event(
        timestamp: pointer.timeStamp.inMicroseconds * 1000,
        data: Data.withPointerSample(sample),
        traceFlowId: pointer.pointer,
      );
      events.add(injectorEvent);
    }

    try {
      return _device.inject(events);
    } catch (e) {
      log.warning('Failed to dispatch pointer events: $e');
    }
  }

  // Check if [PointerEvent] is one of supported events.
  bool _isValidPointerEvent(PointerEvent pointer) {
    return pointer != null &&
        (pointer is PointerDownEvent ||
            pointer is PointerUpEvent ||
            pointer is PointerMoveEvent);
  }

  List<Float32List> _extentFromRect(Rect rect) => [
        Float32List.fromList([0, 0]),
        Float32List.fromList([rect.width, rect.height]),
      ];

  Float32List _transformFromRect(Rect rect) => Float32List.fromList(<double>[
        1, 0, 0, // first column
        0, 1, 0, // second column
        rect.left, rect.top, 1, // third column
      ]);
}
