// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

/// The Web implementation of [ui.FlutterWindow].
class EngineFlutterWindow extends ui.FlutterWindow {
  EngineFlutterWindow({Object windowId, this.platformDispatcher})
    : _windowId = windowId;

  final Object _windowId;
  final ui.PlatformDispatcher platformDispatcher;

  @override
  ui.ViewConfiguration get viewConfiguration {
    final EnginePlatformDispatcher engineDispatcher = platformDispatcher as EnginePlatformDispatcher;
    assert(engineDispatcher._windowConfigurations.containsKey(_windowId));
    return engineDispatcher._windowConfigurations[_windowId];
  }
}

/// The Web implementation of [ui.SingletonFlutterWindow].
class EngineSingletonFlutterWindow extends ui.SingletonFlutterWindow {
  EngineSingletonFlutterWindow({Object windowId, this.platformDispatcher})
      : _windowId = windowId {
    final EnginePlatformDispatcher engineDispatcher = platformDispatcher as EnginePlatformDispatcher;
    final ui.Screen newScreen = EngineScreen(0, platformDispatcher);
    engineDispatcher._screens[0] = newScreen;
    engineDispatcher._screenConfigurations[0] = ui.ScreenConfiguration();
    engineDispatcher._windows[windowId] = this;
    engineDispatcher._windowConfigurations[windowId] = ui.ViewConfiguration(screen: newScreen);
  }

  final Object _windowId;

  final ui.PlatformDispatcher platformDispatcher;

  @override
  ui.ViewConfiguration get viewConfiguration {
    final EnginePlatformDispatcher engineDispatcher = platformDispatcher as EnginePlatformDispatcher;
    assert(engineDispatcher._windowConfigurations.containsKey(_windowId));
    return engineDispatcher._windowConfigurations[_windowId];
  }

  @override
  double get devicePixelRatio => _debugDevicePixelRatio != null
      ? _debugDevicePixelRatio
      : EnginePlatformDispatcher.browserDevicePixelRatio;

  /// Overrides the default device pixel ratio.
  ///
  /// This is useful in tests to emulate screens of different dimensions.
  void debugOverrideDevicePixelRatio(double value) {
    _debugDevicePixelRatio = value;
  }

  double _debugDevicePixelRatio;

  @override
  ui.Size get physicalSize {
    if (_physicalSize == null) {
      _computePhysicalSize();
    }
    assert(_physicalSize != null);
    return _physicalSize;
  }

  /// Computes the physical size of the screen from [html.window].
  ///
  /// This function is expensive. It triggers browser layout if there are
  /// pending DOM writes.
  void _computePhysicalSize() {
    bool override = false;

    assert(() {
      if (webOnlyDebugPhysicalSizeOverride != null) {
        _physicalSize = webOnlyDebugPhysicalSizeOverride;
        override = true;
      }
      return true;
    }());

    if (!override) {
      double windowInnerWidth;
      double windowInnerHeight;
      final html.VisualViewport viewport = html.window.visualViewport;
      if (viewport != null) {
        windowInnerWidth = viewport.width * devicePixelRatio;
        windowInnerHeight = viewport.height * devicePixelRatio;
      } else {
        windowInnerWidth = html.window.innerWidth * devicePixelRatio;
        windowInnerHeight = html.window.innerHeight * devicePixelRatio;
      }
      _physicalSize = ui.Size(
        windowInnerWidth,
        windowInnerHeight,
      );
    }
  }

  void computeOnScreenKeyboardInsets() {
    double windowInnerHeight;
    final html.VisualViewport viewport = html.window.visualViewport;
    if (viewport != null) {
      windowInnerHeight = viewport.height * devicePixelRatio;
    } else {
      windowInnerHeight = html.window.innerHeight * devicePixelRatio;
    }
    final double bottomPadding = _physicalSize.height - windowInnerHeight;
    _viewInsets =
        WindowPadding(bottom: bottomPadding, left: 0, right: 0, top: 0);
  }

  /// Uses the previous physical size and current innerHeight/innerWidth
  /// values to decide if a device is rotating.
  ///
  /// During a rotation the height and width values will (almost) swap place.
  /// Values can slightly differ due to space occupied by the browser header.
  /// For example the following values are collected for Pixel 3 rotation:
  ///
  /// height: 658 width: 393
  /// new height: 313 new width: 738
  ///
  /// The following values are from a changed caused by virtual keyboard.
  ///
  /// height: 658 width: 393
  /// height: 368 width: 393
  bool isRotation() {
    double height = 0;
    double width = 0;
    if (html.window.visualViewport != null) {
      height = html.window.visualViewport.height * devicePixelRatio;
      width = html.window.visualViewport.width * devicePixelRatio;
    } else {
      height = html.window.innerHeight * devicePixelRatio;
      width = html.window.innerWidth * devicePixelRatio;
    }
    // First confirm both height and width are affected.
    if (_physicalSize.height != height && _physicalSize.width != width) {
      // If prior to rotation height is bigger than width it should be the
      // opposite after the rotation and vice versa.
      if ((_physicalSize.height > _physicalSize.width && height < width) ||
          (_physicalSize.width > _physicalSize.height && width < height)) {
        // Rotation detected
        return true;
      }
    }
    return false;
  }

  @override
  WindowPadding get viewInsets => _viewInsets;
  WindowPadding _viewInsets = ui.WindowPadding.zero;

  /// Lazily populated and cleared at the end of the frame.
  ui.Size _physicalSize;

  /// Overrides the value of [physicalSize] in tests.
  ui.Size webOnlyDebugPhysicalSizeOverride;

  ui.VoidCallback/*?*/ get onMetricsChanged => platformDispatcher.onMetricsChanged;
  set onMetricsChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onMetricsChanged = callback;
  }

  ui.Locale get locale => platformDispatcher.locale;
  List<ui.Locale> get locales => platformDispatcher.locales;

  ui.Locale/*?*/ computePlatformResolvedLocale(List<ui.Locale> supportedLocales) {
    // TODO(garyq): Implement on web.
    return null;
  }

  ui.VoidCallback/*?*/ get onLocaleChanged => platformDispatcher.onLocaleChanged;
  set onLocaleChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onLocaleChanged = callback;
  }

  double get textScaleFactor => platformDispatcher.textScaleFactor;
  bool get alwaysUse24HourFormat => platformDispatcher.alwaysUse24HourFormat;

  ui.VoidCallback/*?*/ get onTextScaleFactorChanged => platformDispatcher.onTextScaleFactorChanged;
  set onTextScaleFactorChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onTextScaleFactorChanged = callback;
  }

  ui.Brightness get platformBrightness => platformDispatcher.platformBrightness;

  ui.VoidCallback/*?*/ get onPlatformBrightnessChanged => platformDispatcher.onPlatformBrightnessChanged;
  set onPlatformBrightnessChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onPlatformBrightnessChanged = callback;
  }

  ui.FrameCallback/*?*/ get onBeginFrame => platformDispatcher.onBeginFrame;
  set onBeginFrame(ui.FrameCallback/*?*/ callback) {
    platformDispatcher.onBeginFrame = callback;
  }

  ui.TimingsCallback/*?*/ get onReportTimings => platformDispatcher.onReportTimings;
  set onReportTimings(ui.TimingsCallback/*?*/ callback) {
    platformDispatcher.onReportTimings = callback;
  }

  ui.VoidCallback/*?*/ get onDrawFrame => platformDispatcher.onDrawFrame;
  set onDrawFrame(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onDrawFrame = callback;
  }

  ui.PointerDataPacketCallback/*?*/ get onPointerDataPacket => platformDispatcher.onPointerDataPacket;
  set onPointerDataPacket(ui.PointerDataPacketCallback/*?*/ callback) {
    platformDispatcher.onPointerDataPacket = callback;
  }

  String get defaultRouteName => platformDispatcher.initialRouteName;

  bool get semanticsEnabled => EngineSemanticsOwner.instance.semanticsEnabled;

  ui.VoidCallback/*?*/ get onSemanticsEnabledChanged => platformDispatcher.onSemanticsEnabledChanged;
  set onSemanticsEnabledChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onSemanticsEnabledChanged = callback;
  }

  ui.SemanticsActionCallback/*?*/ get onSemanticsAction => platformDispatcher.onSemanticsAction;
  set onSemanticsAction(ui.SemanticsActionCallback/*?*/ callback) {
    platformDispatcher.onSemanticsAction = callback;
  }

  ui.VoidCallback/*?*/ get onAccessibilityFeaturesChanged => platformDispatcher.onAccessibilityFeaturesChanged;
  set onAccessibilityFeaturesChanged(ui.VoidCallback/*?*/ callback) {
    platformDispatcher.onAccessibilityFeaturesChanged = callback;
  }

  ui.PlatformMessageCallback/*?*/ get onPlatformMessage => platformDispatcher.onPlatformMessage;
  set onPlatformMessage(ui.PlatformMessageCallback/*?*/ callback) {
    platformDispatcher.onPlatformMessage = callback;
  }

  void sendPlatformMessage(
      String name,
      ByteData/*?*/ data,
      ui.PlatformMessageResponseCallback/*?*/ callback,
      ) {
    platformDispatcher.sendPlatformMessage(name, data, callback);
  }

  void updateSemantics(ui.SemanticsUpdate update) => platformDispatcher.updateSemantics(update);

  ui.AccessibilityFeatures get accessibilityFeatures => platformDispatcher.accessibilityFeatures;

  void render(ui.Scene scene) => platformDispatcher.render(scene, this);

  String get initialLifecycleState => 'AppLifecycleState.resumed';
}

/// The window singleton.
///
/// `dart:ui` window delegates to this value. However, this value has a wider
/// API surface, providing Web-specific functionality that the standard
/// `dart:ui` version does not.
final EngineSingletonFlutterWindow/*!*/ window = EngineSingletonFlutterWindow(windowId: 0, platformDispatcher: EnginePlatformDispatcher.instance);

/// The Web implementation of [ui.WindowPadding].
class WindowPadding implements ui.WindowPadding {
  const WindowPadding({
    this.left,
    this.top,
    this.right,
    this.bottom,
  });

  final double left;
  final double top;
  final double right;
  final double bottom;
}
