// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of ui;

typedef PlatformConfigurationChangedCallback = void Function(PlatformConfiguration configuration);
typedef ViewCreatedCallback = void Function(FlutterView view);
typedef ViewDisposedCallback = void Function(FlutterView view);

abstract class PlatformDispatcher {
  static PlatformDispatcher/*!*/ get instance => engine.EnginePlatformDispatcher.instance;

  PlatformConfiguration/*!*/ get configuration;
  VoidCallback/*?*/ get onPlatformConfigurationChanged;
  set onPlatformConfigurationChanged(VoidCallback/*?*/ callback);

  Iterable<Screen/*!*/>/*!*/ get screens;

  Iterable<FlutterView/*!*/>/*!*/ get views;
  ViewCreatedCallback/*?*/ get onViewCreated;
  set onViewCreated(ViewCreatedCallback/*?*/ callback);
  ViewDisposedCallback/*?*/ get onViewDisposed;
  set onViewDisposed(ViewDisposedCallback/*?*/ callback);
  Future<FlutterView/*?*/> createView(ViewConfigurationRequest/*!*/ request);
  Future<void> configureView(FlutterView/*!*/ view, ViewConfigurationRequest/*!*/ configuration);
  Future<void> disposeView(FlutterView/*!*/ view);

  VoidCallback/*?*/ get onMetricsChanged;
  set onMetricsChanged(VoidCallback/*?*/ callback);

  FrameCallback/*?*/ get onBeginFrame;
  set onBeginFrame(FrameCallback/*?*/ callback);

  VoidCallback/*?*/ get onDrawFrame;
  set onDrawFrame(VoidCallback/*?*/ callback);

  PointerDataPacketCallback/*?*/ get onPointerDataPacket;
  set onPointerDataPacket(PointerDataPacketCallback/*?*/ callback);

  TimingsCallback/*?*/ get onReportTimings;
  set onReportTimings(TimingsCallback/*?*/ callback);

  void sendPlatformMessage(
      String/*!*/ name,
      ByteData/*?*/ data,
      PlatformMessageResponseCallback/*?*/ callback,
  );

  PlatformMessageCallback/*?*/ get onPlatformMessage;
  set onPlatformMessage(PlatformMessageCallback/*?*/ callback);

  void scheduleFrame();

  void render(Scene/*!*/ scene, [FlutterView/*!*/ view]);

  AccessibilityFeatures/*!*/ get accessibilityFeatures;

  VoidCallback/*?*/ get onAccessibilityFeaturesChanged;
  set onAccessibilityFeaturesChanged(VoidCallback/*?*/ callback);

  void updateSemantics(SemanticsUpdate/*!*/ update);

  Locale/*!*/ get locale;

  List<Locale/*!*/>/*!*/ get locales => configuration.locales;

  Locale/*!*/ get platformResolvedLocale => configuration.platformResolvedLocale;

  VoidCallback/*?*/ get onLocaleChanged;
  set onLocaleChanged(VoidCallback/*?*/ callback);

  bool/*!*/ get alwaysUse24HourFormat => configuration.alwaysUse24HourFormat;

  double/*!*/ get textScaleFactor => configuration.textScaleFactor;

  VoidCallback/*?*/ get onTextScaleFactorChanged;
  set onTextScaleFactorChanged(VoidCallback/*?*/ callback);

  Brightness/*!*/ get platformBrightness => configuration.platformBrightness;

  VoidCallback/*?*/ get onPlatformBrightnessChanged;
  set onPlatformBrightnessChanged(VoidCallback/*?*/ callback);

  bool/*!*/ get semanticsEnabled => configuration.semanticsEnabled;

  VoidCallback/*?*/ get onSemanticsEnabledChanged;
  set onSemanticsEnabledChanged(VoidCallback/*?*/ callback);

  SemanticsActionCallback/*?*/ get onSemanticsAction;
  set onSemanticsAction(SemanticsActionCallback/*?*/ callback);

  String/*!*/ get initialRouteName;

  void setIsolateDebugName(String/*!*/ name) {}

  ByteData/*?*/ getPersistentIsolateData() => null;

  String/*?*/ get initialLifecycleState;
}

class PlatformConfiguration {
  const PlatformConfiguration({
    this.accessibilityFeatures = const AccessibilityFeatures._(0),
    this.alwaysUse24HourFormat = false,
    this.semanticsEnabled = false,
    this.platformBrightness = Brightness.light,
    this.textScaleFactor = 1.0,
    this.locales = const <Locale>[],
    this.platformResolvedLocale,
    this.initialRouteName,
  })  : assert(accessibilityFeatures != null),
        assert(alwaysUse24HourFormat != null),
        assert(semanticsEnabled != null),
        assert(platformBrightness != null),
        assert(textScaleFactor != null),
        assert(locales != null);

  PlatformConfiguration copyWith({
    AccessibilityFeatures/*?*/ accessibilityFeatures,
    bool/*?*/ alwaysUse24HourFormat,
    bool/*?*/ semanticsEnabled,
    Brightness/*?*/ platformBrightness,
    double/*?*/ textScaleFactor,
    List<Locale/*!*/>/*?*/ locales,
    Locale/*?*/ platformResolvedLocale,
    String/*?*/ initialRouteName,
  }) {
    return PlatformConfiguration(
      accessibilityFeatures: accessibilityFeatures ?? this.accessibilityFeatures,
      alwaysUse24HourFormat: alwaysUse24HourFormat ?? this.alwaysUse24HourFormat,
      semanticsEnabled: semanticsEnabled ?? this.semanticsEnabled,
      platformBrightness: platformBrightness ?? this.platformBrightness,
      textScaleFactor: textScaleFactor ?? this.textScaleFactor,
      locales: locales ?? this.locales,
      platformResolvedLocale: platformResolvedLocale ?? this.platformResolvedLocale,
      initialRouteName: initialRouteName ?? this.initialRouteName,
    );
  }

  final AccessibilityFeatures/*!*/ accessibilityFeatures;
  final bool/*!*/ alwaysUse24HourFormat;
  final bool/*!*/ semanticsEnabled;
  final Brightness/*!*/ platformBrightness;
  final double/*!*/ textScaleFactor;
  final List<Locale/*!*/>/*!*/ locales;
  final Locale/*?*/ platformResolvedLocale;
  final String/*?*/ initialRouteName;
}

class ScreenConfiguration {
  const ScreenConfiguration({
    this.screenName = '',
    this.geometry = Rect.zero,
    this.devicePixelRatio = 1.0,
    this.viewInsets = WindowPadding.zero,
    this.viewPadding = WindowPadding.zero,
    this.systemGestureInsets = WindowPadding.zero,
    this.padding = WindowPadding.zero,
  })  : assert(screenName != null),
        assert(geometry != null),
        assert(devicePixelRatio != null),
        assert(viewInsets != null),
        assert(viewPadding != null),
        assert(systemGestureInsets != null),
        assert(padding != null);

  ScreenConfiguration copyWith({
    String/*?*/ screenName,
    Rect/*?*/ geometry,
    double/*?*/ devicePixelRatio,
    WindowPadding/*?*/ viewInsets,
    WindowPadding/*?*/ viewPadding,
    WindowPadding/*?*/ systemGestureInsets,
    WindowPadding/*?*/ padding,
  }) {
    return ScreenConfiguration(
      screenName: screenName ?? this.screenName,
      geometry: geometry ?? this.geometry,
      devicePixelRatio: devicePixelRatio ?? this.devicePixelRatio,
      viewInsets: viewInsets ?? this.viewInsets,
      viewPadding: viewPadding ?? this.viewPadding,
      systemGestureInsets: systemGestureInsets ?? this.systemGestureInsets,
      padding: padding ?? this.padding,
    );
  }

  final String/*!*/ screenName;
  final Rect/*!*/ geometry;
  final double/*!*/ devicePixelRatio;
  final WindowPadding/*!*/ viewInsets;
  final WindowPadding/*!*/ viewPadding;
  final WindowPadding/*!*/ systemGestureInsets;
  final WindowPadding/*!*/ padding;
}


class ViewConfigurationRequest {
  const ViewConfigurationRequest({
    this.screen,
    this.geometry,
    this.visible,
    this.order,
    this.orderView,
  }) : assert(orderView != null || (order != ViewOrder.aboveOther && order != ViewOrder.belowOther)),
        assert(orderView == null || (order != ViewOrder.top && order != ViewOrder.bottom)),
        assert(screen != null || geometry != null || order != null || visible != null, 'At least one parameter must be non-null');

  ViewConfigurationRequest copyWith({
    Screen/*?*/ screen,
    Rect/*?*/ geometry,
    bool/*?*/ visible,
    ViewOrder/*?*/ order,
    FlutterView/*?*/ orderView,
  }) {
    return ViewConfigurationRequest(
      screen: screen ?? this.screen,
      geometry: geometry ?? this.geometry,
      visible: visible ?? this.visible,
      order: order ?? this.order,
      orderView: orderView ?? this.orderView,
    );
  }

  final Screen/*?*/ screen;
  final Rect/*?*/ geometry;
  final bool/*?*/ visible;
  final ViewOrder/*?*/ order;
  final FlutterView/*?*/ orderView;

  @override
  String toString() {
    return '$runtimeType[screen: $screen, geometry: $geometry, order: $order]';
  }
}

enum ViewOrder {
  aboveOther,

  belowOther,

  top,

  bottom,
}

class ViewConfiguration {
  const ViewConfiguration({
    this.screen,
    this.window,
    this.geometry = Rect.zero,
    this.depth = double.maxFinite,
    this.visible = false,
    this.viewInsets = WindowPadding.zero,
    this.viewPadding = WindowPadding.zero,
    this.systemGestureInsets = WindowPadding.zero,
    this.padding = WindowPadding.zero,
  })  : assert(screen != null),
        assert(geometry != null),
        assert(depth != null),
        assert(visible != null),
        assert(viewInsets != null),
        assert(viewPadding != null),
        assert(systemGestureInsets != null),
        assert(padding != null);

  ViewConfiguration copyWith({
    Screen/*?*/ screen,
    FlutterWindow/*?*/ window,
    Rect/*?*/ geometry,
    double/*?*/ depth,
    bool/*?*/ visible,
    WindowPadding/*?*/ viewInsets,
    WindowPadding/*?*/ viewPadding,
    WindowPadding/*?*/ systemGestureInsets,
    WindowPadding/*?*/ padding,
  }) {
    return ViewConfiguration(
      screen: screen ?? this.screen,
      window: window ?? this.window,
      geometry: geometry ?? this.geometry,
      depth: depth ?? this.depth,
      visible: visible ?? this.visible,
      viewInsets: viewInsets ?? this.viewInsets,
      viewPadding: viewPadding ?? this.viewPadding,
      systemGestureInsets: systemGestureInsets ?? this.systemGestureInsets,
      padding: padding ?? this.padding,
    );
  }

  final Screen/*!*/ screen;
  final FlutterWindow/*?*/ window;
  final Rect/*!*/ geometry;
  final double/*!*/ depth;
  final bool/*!*/ visible;
  final WindowPadding/*!*/ viewInsets;
  final WindowPadding/*!*/ viewPadding;
  final WindowPadding/*!*/ systemGestureInsets;
  final WindowPadding/*!*/ padding;

  @override
  String toString() {
    return '$runtimeType[screen: $screen, window: $window, geometry: $geometry, depth: $depth]';
  }
}
