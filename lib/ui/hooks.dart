// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(dnfield): Remove unused_import ignores when https://github.com/dart-lang/sdk/issues/35164 is resolved.

// @dart = 2.9

part of dart.ui;

@pragma('vm:entry-point')
// ignore: unused_element
void _updateWindowMetrics(
  Object id,
  Object screenId,
  double left,
  double top,
  double width,
  double height,
  double depth,
  double viewPaddingTop,
  double viewPaddingRight,
  double viewPaddingBottom,
  double viewPaddingLeft,
  double viewInsetTop,
  double viewInsetRight,
  double viewInsetBottom,
  double viewInsetLeft,
  double systemGestureInsetTop,
  double systemGestureInsetRight,
  double systemGestureInsetBottom,
  double systemGestureInsetLeft,
) {
  PlatformDispatcher.instance._updateViewMetrics(
    id,
    screenId,
    left,
    top,
    width,
    height,
    depth,
    viewPaddingTop,
    viewPaddingRight,
    viewPaddingBottom,
    viewPaddingLeft,
    viewInsetTop,
    viewInsetRight,
    viewInsetBottom,
    viewInsetLeft,
    systemGestureInsetTop,
    systemGestureInsetRight,
    systemGestureInsetBottom,
    systemGestureInsetLeft,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateScreenMetrics(
  Object id,
  String screenName,
  double left,
  double top,
  double width,
  double height,
  double devicePixelRatio,
  double viewPaddingTop,
  double viewPaddingRight,
  double viewPaddingBottom,
  double viewPaddingLeft,
  double viewInsetTop,
  double viewInsetRight,
  double viewInsetBottom,
  double viewInsetLeft,
  double systemGestureInsetTop,
  double systemGestureInsetRight,
  double systemGestureInsetBottom,
  double systemGestureInsetLeft,
) {
  PlatformDispatcher.instance._updateScreenMetrics(
    id,
    screenName,
    left,
    top,
    width,
    height,
    devicePixelRatio,
    viewPaddingTop,
    viewPaddingRight,
    viewPaddingBottom,
    viewPaddingLeft,
    viewInsetTop,
    viewInsetRight,
    viewInsetBottom,
    viewInsetLeft,
    systemGestureInsetTop,
    systemGestureInsetRight,
    systemGestureInsetBottom,
    systemGestureInsetLeft,
  );
}

typedef _LocaleClosure = String? Function();

String? _localeClosure() {
  if (PlatformDispatcher.instance.configuration.platformResolvedLocale == null) {
    return null;
  }
  return PlatformDispatcher.instance.configuration.platformResolvedLocale.toString();
}

@pragma('vm:entry-point')
// ignore: unused_element
_LocaleClosure? _getLocaleClosure() => _localeClosure;

@pragma('vm:entry-point')
// ignore: unused_element
void _updateLocales(List<String> locales) {
  PlatformDispatcher.instance._updateLocales(locales);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateUserSettingsData(String jsonData) {
  final Map<String, dynamic> data = json.decode(jsonData) as Map<String, dynamic>;
  if (data.isEmpty) {
    return;
  }
  PlatformDispatcher.instance._updateUserSettingsData(data);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateLifecycleState(String state) {
  PlatformDispatcher.instance._updateLifecycleState(state);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateSemanticsEnabled(bool enabled) {
  PlatformDispatcher.instance._updateSemanticsEnabled(enabled);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateAccessibilityFeatures(int values) {
  final AccessibilityFeatures newFeatures = AccessibilityFeatures._(values);
  PlatformDispatcher.instance._updateAccessibilityFeatures(newFeatures);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _dispatchPlatformMessage(String name, ByteData? data, int responseId) {
  PlatformDispatcher.instance._dispatchPlatformMessage(name, data, responseId);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _dispatchPointerDataPacket(ByteData packet) {
  if (PlatformDispatcher.instance.onPointerDataPacket != null)
    _invoke1<PointerDataPacket>(
      PlatformDispatcher.instance.onPointerDataPacket,
      PlatformDispatcher.instance._onPointerDataPacketZone,
      _unpackPointerDataPacket(packet),
    );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _dispatchSemanticsAction(int id, int action, ByteData? args) {
  _invoke3<int, SemanticsAction, ByteData?>(
    PlatformDispatcher.instance.onSemanticsAction,
    PlatformDispatcher.instance._onSemanticsActionZone,
    id,
    SemanticsAction.values[action]!,
    args,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _beginFrame(int microseconds) {
  _invoke1<Duration>(
    PlatformDispatcher.instance.onBeginFrame,
    PlatformDispatcher.instance._onBeginFrameZone,
    Duration(microseconds: microseconds),
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _reportTimings(List<int> timings) {
  assert(timings.length % FramePhase.values.length == 0);
  final List<FrameTiming> frameTimings = <FrameTiming>[];
  for (int i = 0; i < timings.length; i += FramePhase.values.length) {
    frameTimings.add(FrameTiming(timings.sublist(i, i + FramePhase.values.length)));
  }
  _invoke1(
    PlatformDispatcher.instance.onReportTimings,
    PlatformDispatcher.instance._onReportTimingsZone,
    frameTimings,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _drawFrame() {
  _invoke(
    PlatformDispatcher.instance.onDrawFrame,
    PlatformDispatcher.instance._onDrawFrameZone,
  );
}

// ignore: always_declare_return_types, prefer_generic_function_type_aliases
typedef _UnaryFunction(Null args);
// ignore: always_declare_return_types, prefer_generic_function_type_aliases
typedef _BinaryFunction(Null args, Null message);

@pragma('vm:entry-point')
// ignore: unused_element
void _runMainZoned(
  Function startMainIsolateFunction,
  Function userMainFunction,
  List<String> args,
) {
  startMainIsolateFunction(() {
    runZonedGuarded<void>(() {
      if (userMainFunction is _BinaryFunction) {
        // This seems to be undocumented but supported by the command line VM.
        // Let's do the same in case old entry-points are ported to Flutter.
        (userMainFunction as dynamic)(args, '');
      } else if (userMainFunction is _UnaryFunction) {
        (userMainFunction as dynamic)(args);
      } else {
        userMainFunction();
      }
    }, (Object error, StackTrace stackTrace) {
      _reportUnhandledException(error.toString(), stackTrace.toString());
    });
  }, null);
}

void _reportUnhandledException(String error, String stackTrace)
    native 'PlatformConfiguration_reportUnhandledException';

/// Invokes [callback] inside the given [zone].
void _invoke(void callback()?, Zone zone) {
  if (callback == null) {
    return;
  }

  assert(zone != null); // ignore: unnecessary_null_comparison

  if (identical(zone, Zone.current)) {
    callback();
  } else {
    zone.runGuarded(callback);
  }
}

/// Invokes [callback] inside the given [zone] passing it [arg].
void _invoke1<A>(void callback(A a)?, Zone zone, A arg) {
  if (callback == null) {
    return;
  }

  assert(zone != null); // ignore: unnecessary_null_comparison

  if (identical(zone, Zone.current)) {
    callback(arg);
  } else {
    zone.runUnaryGuarded<A>(callback, arg);
  }
}

/// Invokes [callback] inside the given [zone] passing it [arg1], [arg2], and [arg3].
void _invoke3<A1, A2, A3>(
  void callback(A1 a1, A2 a2, A3 a3)?,
  Zone zone,
  A1 arg1,
  A2 arg2,
  A3 arg3,
) {
  if (callback == null) {
    return;
  }

  assert(zone != null); // ignore: unnecessary_null_comparison

  if (identical(zone, Zone.current)) {
    callback(arg1, arg2, arg3);
  } else {
    zone.runGuarded(() {
      callback(arg1, arg2, arg3);
    });
  }
}

// If this value changes, update the encoding code in the following files:
//
//  * pointer_data.cc
//  * pointers.dart
//  * AndroidTouchProcessor.java
const int _kPointerDataFieldCount = 28;

PointerDataPacket _unpackPointerDataPacket(ByteData packet) {
  const int kStride = Int64List.bytesPerElement;
  const int kBytesPerPointerData = _kPointerDataFieldCount * kStride;
  final int length = packet.lengthInBytes ~/ kBytesPerPointerData;
  assert(length * kBytesPerPointerData == packet.lengthInBytes);
  final List<PointerData> data = <PointerData>[];
  for (int i = 0; i < length; ++i) {
    int offset = i * _kPointerDataFieldCount;
    data.add(PointerData(
      timeStamp: Duration(microseconds: packet.getInt64(kStride * offset++, _kFakeHostEndian)),
      change: PointerChange.values[packet.getInt64(kStride * offset++, _kFakeHostEndian)],
      kind: PointerDeviceKind.values[packet.getInt64(kStride * offset++, _kFakeHostEndian)],
      signalKind: PointerSignalKind.values[packet.getInt64(kStride * offset++, _kFakeHostEndian)],
      device: packet.getInt64(kStride * offset++, _kFakeHostEndian),
      pointerIdentifier: packet.getInt64(kStride * offset++, _kFakeHostEndian),
      physicalX: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      physicalY: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      physicalDeltaX: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      physicalDeltaY: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      buttons: packet.getInt64(kStride * offset++, _kFakeHostEndian),
      obscured: packet.getInt64(kStride * offset++, _kFakeHostEndian) != 0,
      synthesized: packet.getInt64(kStride * offset++, _kFakeHostEndian) != 0,
      pressure: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      pressureMin: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      pressureMax: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      distance: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      distanceMax: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      size: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      radiusMajor: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      radiusMinor: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      radiusMin: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      radiusMax: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      orientation: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      tilt: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      platformData: packet.getInt64(kStride * offset++, _kFakeHostEndian),
      scrollDeltaX: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
      scrollDeltaY: packet.getFloat64(kStride * offset++, _kFakeHostEndian),
    ));
    assert(offset == (i + 1) * _kPointerDataFieldCount);
  }
  return PointerDataPacket(data: data);
}
