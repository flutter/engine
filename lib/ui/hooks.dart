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
  assert(PlatformDispatcher.instance._screens[screenId] != null);
  final ViewConfiguration previousConfiguration =
      PlatformDispatcher.instance._viewConfigurations[id] ?? ViewConfiguration(screen: PlatformDispatcher.instance._screens[screenId]);
  PlatformDispatcher.instance._viewConfigurations[id] = previousConfiguration.copyWith(
    screen: PlatformDispatcher.instance._screens[screenId],
    geometry: Rect.fromLTWH(left, top, width, height),
    depth: depth,
    viewPadding: WindowPadding._(
      top: viewPaddingTop,
      right: viewPaddingRight,
      bottom: viewPaddingBottom,
      left: viewPaddingLeft,
    ),
    viewInsets: WindowPadding._(
      top: viewInsetTop,
      right: viewInsetRight,
      bottom: viewInsetBottom,
      left: viewInsetLeft,
    ),
    padding: WindowPadding._(
      top: math.max(0.0, viewPaddingTop - viewInsetTop),
      right: math.max(0.0, viewPaddingRight - viewInsetRight),
      bottom: math.max(0.0, viewPaddingBottom - viewInsetBottom),
      left: math.max(0.0, viewPaddingLeft - viewInsetLeft),
    ),
    systemGestureInsets: WindowPadding._(
      top: math.max(0.0, systemGestureInsetTop),
      right: math.max(0.0, systemGestureInsetRight),
      bottom: math.max(0.0, systemGestureInsetBottom),
      left: math.max(0.0, systemGestureInsetLeft),
    ),
  );
  if (!PlatformDispatcher.instance._views.containsKey(id)) {
    PlatformDispatcher.instance._views[id] = FlutterWindow._(windowId: id, platformDispatcher: PlatformDispatcher.instance);
  }
  _invoke(
    PlatformDispatcher.instance.onMetricsChanged,
    PlatformDispatcher.instance._onMetricsChangedZone,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateScreenMetrics(
  Object/*!*/ id,
  String/*!*/ displayName,
  double/*!*/ left,
  double/*!*/ top,
  double/*!*/ width,
  double/*!*/ height,
  double/*!*/ devicePixelRatio,
  double/*!*/ viewPaddingTop,
  double/*!*/ viewPaddingRight,
  double/*!*/ viewPaddingBottom,
  double/*!*/ viewPaddingLeft,
  double/*!*/ viewInsetTop,
  double/*!*/ viewInsetRight,
  double/*!*/ viewInsetBottom,
  double/*!*/ viewInsetLeft,
  double/*!*/ systemGestureInsetTop,
  double/*!*/ systemGestureInsetRight,
  double/*!*/ systemGestureInsetBottom,
  double/*!*/ systemGestureInsetLeft,
) {
  final ScreenConfiguration previousConfiguration =
      PlatformDispatcher.instance._screenConfigurations[id] ?? const ScreenConfiguration();
  PlatformDispatcher.instance._screenConfigurations[id] = previousConfiguration.copyWith(
    screenName: displayName,
    geometry: Rect.fromLTWH(left, top, width, height),
    devicePixelRatio: devicePixelRatio,
    viewPadding: WindowPadding._(
      top: viewPaddingTop,
      right: viewPaddingRight,
      bottom: viewPaddingBottom,
      left: viewPaddingLeft,
    ),
    viewInsets: WindowPadding._(
      top: viewInsetTop,
      right: viewInsetRight,
      bottom: viewInsetBottom,
      left: viewInsetLeft,
    ),
    padding: WindowPadding._(
      top: math.max(0.0, viewPaddingTop - viewInsetTop),
      right: math.max(0.0, viewPaddingRight - viewInsetRight),
      bottom: math.max(0.0, viewPaddingBottom - viewInsetBottom),
      left: math.max(0.0, viewPaddingLeft - viewInsetLeft),
    ),
    systemGestureInsets: WindowPadding._(
      top: math.max(0.0, systemGestureInsetTop),
      right: math.max(0.0, systemGestureInsetRight),
      bottom: math.max(0.0, systemGestureInsetBottom),
      left: math.max(0.0, systemGestureInsetLeft),
    ),
  );
  if (!PlatformDispatcher.instance._screens.containsKey(id)) {
    PlatformDispatcher.instance._screens[id] = Screen._(screenId: id, platformDispatcher: PlatformDispatcher.instance);
  }
  _invoke(
    PlatformDispatcher.instance.onMetricsChanged,
    PlatformDispatcher.instance._onMetricsChangedZone,
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
  const int stringsPerLocale = 4;
  final int numLocales = locales.length ~/ stringsPerLocale;
  final PlatformConfiguration previousConfiguration = PlatformDispatcher.instance.configuration;
  final List<Locale> newLocales = <Locale>[];
  bool localesDiffer = numLocales != previousConfiguration.locales.length;
  for (int localeIndex = 0; localeIndex < numLocales; localeIndex++) {
    final String countryCode = locales[localeIndex * stringsPerLocale + 1];
    final String scriptCode = locales[localeIndex * stringsPerLocale + 2];

    newLocales.add(Locale.fromSubtags(
      languageCode: locales[localeIndex * stringsPerLocale],
      countryCode: countryCode.isEmpty ? null : countryCode,
      scriptCode: scriptCode.isEmpty ? null : scriptCode,
    ));
    if (!localesDiffer && newLocales.last != previousConfiguration.locales[localeIndex]) {
      localesDiffer = true;
    }
  }
  if (!localesDiffer) {
    return;
  }
  PlatformDispatcher.instance._configuration = previousConfiguration.copyWith(
    locales: newLocales,
  );
  _invoke(
    PlatformDispatcher.instance.onPlatformConfigurationChanged,
    PlatformDispatcher.instance._onPlatformConfigurationChangedZone,
  );
  _invoke(
    PlatformDispatcher.instance.onLocaleChanged,
    PlatformDispatcher.instance._onLocaleChangedZone,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateUserSettingsData(String jsonData) {
  final Map<String, dynamic> data = json.decode(jsonData) as Map<String, dynamic>;
  if (data.isEmpty) {
    return;
  }

  final double textScaleFactor = (data['textScaleFactor'] as num).toDouble();
  final bool alwaysUse24HourFormat = data['alwaysUse24HourFormat'] as bool;
  final Brightness platformBrightness =
      data['platformBrightness'] as String == 'dark' ? Brightness.dark : Brightness.light;
  final PlatformConfiguration previousConfiguration = PlatformDispatcher.instance.configuration;
  final bool platformBrightnessChanged =
      previousConfiguration.platformBrightness != platformBrightness;
  final bool textScaleFactorChanged = previousConfiguration.textScaleFactor != textScaleFactor;
  final bool alwaysUse24HourFormatChanged =
      previousConfiguration.alwaysUse24HourFormat != alwaysUse24HourFormat;
  if (!platformBrightnessChanged && !textScaleFactorChanged && !alwaysUse24HourFormatChanged) {
    return;
  }
  PlatformDispatcher.instance._configuration = previousConfiguration.copyWith(
    textScaleFactor: textScaleFactor,
    alwaysUse24HourFormat: alwaysUse24HourFormat,
    platformBrightness: platformBrightness,
  );
  _invoke(
    PlatformDispatcher.instance.onPlatformConfigurationChanged,
    PlatformDispatcher.instance._onPlatformConfigurationChangedZone,
  );
  if (textScaleFactorChanged) {
    _invoke(
      PlatformDispatcher.instance.onTextScaleFactorChanged,
      PlatformDispatcher.instance._onTextScaleFactorChangedZone,
    );
  }
  if (platformBrightnessChanged) {
    _invoke(
      PlatformDispatcher.instance.onPlatformBrightnessChanged,
      PlatformDispatcher.instance._onPlatformBrightnessChangedZone,
    );
  }
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateLifecycleState(String state) {
  // We do not update the state if the state has already been used to initialize
  // the lifecycleState.
  if (!PlatformDispatcher.instance._initialLifecycleStateAccessed)
    PlatformDispatcher.instance._initialLifecycleState = state;
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateSemanticsEnabled(bool enabled) {
  final PlatformConfiguration previousConfiguration = PlatformDispatcher.instance.configuration;
  if (previousConfiguration.semanticsEnabled == enabled) {
    return;
  }
  PlatformDispatcher.instance._configuration = previousConfiguration.copyWith(
    semanticsEnabled: enabled,
  );
  _invoke(PlatformDispatcher.instance.onPlatformConfigurationChanged,
      PlatformDispatcher.instance._onPlatformConfigurationChangedZone);
  _invoke(PlatformDispatcher.instance.onSemanticsEnabledChanged,
      PlatformDispatcher.instance._onSemanticsEnabledChangedZone);
}

@pragma('vm:entry-point')
// ignore: unused_element
void _updateAccessibilityFeatures(int values) {
  final AccessibilityFeatures newFeatures = AccessibilityFeatures._(values);
  final PlatformConfiguration previousConfiguration = PlatformDispatcher.instance.configuration;
  if (newFeatures == previousConfiguration.accessibilityFeatures) {
    return;
  }
  PlatformDispatcher.instance._configuration = previousConfiguration.copyWith(
    accessibilityFeatures: newFeatures,
  );
  _invoke(
    PlatformDispatcher.instance.onPlatformConfigurationChanged,
    PlatformDispatcher.instance._onPlatformConfigurationChangedZone,
  );
  _invoke(
    PlatformDispatcher.instance.onAccessibilityFeaturesChanged,
    PlatformDispatcher.instance._onAccessibilityFeaturesChangedZone,
  );
}

@pragma('vm:entry-point')
// ignore: unused_element
void _dispatchPlatformMessage(String name, ByteData? data, int responseId) {
  if (name == ChannelBuffers.kControlChannelName) {
    try {
      channelBuffers.handleMessage(data!);
    } catch (ex) {
      _printDebug('Message to "$name" caused exception $ex');
    } finally {
      PlatformDispatcher.instance._respondToPlatformMessage(responseId, null);
    }
  } else if (PlatformDispatcher.instance.onPlatformMessage != null) {
    _invoke3<String, ByteData?, PlatformMessageResponseCallback>(
      PlatformDispatcher.instance.onPlatformMessage,
      PlatformDispatcher.instance._onPlatformMessageZone,
      name,
      data,
      (ByteData? responseData) {
        PlatformDispatcher.instance._respondToPlatformMessage(responseId, responseData);
      },
    );
  } else {
    channelBuffers.push(name, data, (ByteData? responseData) {
      PlatformDispatcher.instance._respondToPlatformMessage(responseId, responseData);
    });
  }
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
    void callback(A1 a1, A2 a2, A3 a3)?, Zone zone, A1 arg1, A2 arg2, A3 arg3) {
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
