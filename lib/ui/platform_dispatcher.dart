// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9
part of dart.ui;

/// Callback type for [PlatformDispatcher.onPlatformConfigurationChanged].
///
/// Receives the new `configuration`.
typedef PlatformConfigurationChangedCallback = void Function(PlatformConfiguration configuration);

/// Platform event dispatcher singleton.
///
/// The most basic interface to the host operating system's interface.
///
/// This is the central entry point for platform messages and configuration
/// events from the platform.
///
/// It exposes the core scheduler API, the input event callback, the graphics
/// drawing API, and other such core services.
///
/// It manages the list of the application's [views] and the [screens] attached
/// to the device, as well as the [configuration] of various platform
/// attributes.
///
/// It is the type of the `WidgetsBinding.instance.platformDispatcher`
/// singleton.
///
/// While there is also a [PlatformDispatcher.instance] singleton object in
/// `dart:ui`, unless you are implementing your own binding, consider avoiding a
/// static reference to it. See the documentation for
/// [PlatformDispatcher.instance] for more details about why you might want to
/// avoid using it directly.
class PlatformDispatcher {
  /// Private constructor, since only dart:ui is supposed to create one of
  /// these.
  PlatformDispatcher._() {
    _setNeedsReportTimings = _nativeSetNeedsReportTimings;
  }

  /// The [PlatformDispatcher] singleton.
  ///
  /// Instead of accessing this directly, consider avoiding a static reference
  /// to this and instead use a binding for dependency resolution such as
  /// `WidgetsBinding.instance.platformDispatcher`.
  ///
  /// Static access of this object means that Flutter has few, if any options to
  /// fake or mock the given object in tests. Even in cases where Dart offers
  /// special language constructs to forcefully shadow such properties, those
  /// mechanisms would only be reasonable for tests and they would not be
  /// reasonable for a future of Flutter where we legitimately want to select an
  /// appropriate implementation at runtime.
  ///
  /// Flutter's binding uses this instance, and if you are implementing your own
  /// binding, of course, it may make sense to use the
  /// [PlatformDispatcher.instance] object directly.
  ///
  /// If you need to access these APIs before invoking `runApp()`, but are using
  /// Flutter bindings, instantiate the bindings yourself by calling
  /// `WidgetsFlutterBinding.ensureInitialized()` and then use
  /// `WidgetsBinding.instance.platformDispatcher`.
  static final PlatformDispatcher instance = PlatformDispatcher._();

  /// The current platform configuration.
  ///
  /// If values in this configuration change, [onMetricsChanged] will be called.
  PlatformConfiguration get configuration => _configuration;
  PlatformConfiguration _configuration = const PlatformConfiguration();

  /// Called when the platform configuration changes.
  VoidCallback? get onPlatformConfigurationChanged => _onPlatformConfigurationChanged;
  VoidCallback? _onPlatformConfigurationChanged;
  Zone _onPlatformConfigurationChangedZone = Zone.root;
  set onPlatformConfigurationChanged(VoidCallback? callback) {
    _onPlatformConfigurationChanged = callback;
    _onPlatformConfigurationChangedZone = Zone.current;
  }

  /// The current list of available screens on the device.
  ///
  /// If the list of screens or their configuration changes, [onMetricsChanged]
  /// will be called.
  Iterable<Screen> get screens => _screens.values;
  Map<Object, Screen> _screens = <Object, Screen>{};

  // A map of opaque platform screen identifiers to screen configurations.
  Map<Object, ScreenConfiguration> _screenConfigurations = <Object, ScreenConfiguration>{};

  /// The current list of views, including top level platform windows used by
  /// the application.
  ///
  /// If the list of views changes, [onViewCreated] or [onViewDisposed] will be
  /// called. If their configurations change, [onMetricsChanged] will be called.
  Iterable<FlutterView> get views => _views.values;
  Map<Object, FlutterView> _views = <Object, FlutterView>{};

  // A map of opaque platform view identifiers to view configurations.
  Map<Object, ViewConfiguration> _viewConfigurations = <Object, ViewConfiguration>{};

  void _updateViewMetrics(
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
    assert(_screens[screenId] != null);
    final ViewConfiguration previousConfiguration = _viewConfigurations[id] ?? ViewConfiguration(_screens[screenId] ?? Screen.invalid);
    _viewConfigurations[id] = previousConfiguration.copyWith(
      screen: _screens[screenId],
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
    if (!_views.containsKey(id)) {
      _views[id] = FlutterWindow._(id, PlatformDispatcher.instance);
    }
    _invoke(onMetricsChanged, _onMetricsChangedZone);
  }

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
    final ScreenConfiguration previousConfiguration =
        _screenConfigurations[id] ?? const ScreenConfiguration();
    _screenConfigurations[id] = previousConfiguration.copyWith(
      screenName: screenName,
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
    if (!_screens.containsKey(id)) {
      _screens[id] = Screen._(id, this);
    }
    _invoke(onMetricsChanged, _onMetricsChangedZone);
  }

  /// A callback that is invoked whenever any [ViewConfiguration] field in the
  /// [views] or [ScreenConfiguration] field in the [screens] changes, or when a
  /// view or screen is added or removed.
  ///
  /// For example when the device is rotated or when the application is resized
  /// (e.g. when showing applications side-by-side on Android).
  ///
  /// The engine invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// The framework registers with this callback and updates the layout
  /// appropriately.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    register for notifications when this is called.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  VoidCallback? get onMetricsChanged => _onMetricsChanged;
  VoidCallback? _onMetricsChanged;
  Zone _onMetricsChangedZone = Zone.root; // ignore: unused_field
  set onMetricsChanged(VoidCallback? callback) {
    _onMetricsChanged = callback;
    _onMetricsChangedZone = Zone.current;
  }

  /// A callback invoked when any view begins a frame.
  ///
  /// {@template flutter.foundation.PlatformDispatcher.onBeginFrame}
  /// A callback that is invoked to notify the application that it is an
  /// appropriate time to provide a scene using the [SceneBuilder] API and the
  /// [PlatformDispatcher.render] method.
  ///
  /// When possible, this is driven by the hardware VSync signal of the attached
  /// screen with the highest VSync rate. This is only called if
  /// [PlatformDispatcher.scheduleFrame] has been called since the last time
  /// this callback was invoked.
  /// {@endtemplate}
  FrameCallback? get onBeginFrame => _onBeginFrame;
  FrameCallback? _onBeginFrame;
  Zone _onBeginFrameZone = Zone.root;
  set onBeginFrame(FrameCallback? callback) {
    _onBeginFrame = callback;
    _onBeginFrameZone = Zone.current;
  }

  /// {@template flutter.foundation.PlatformDispatcher.onDrawFrame}
  /// A callback that is invoked for each frame after [onBeginFrame] has
  /// completed and after the microtask queue has been drained.
  ///
  /// This can be used to implement a second phase of frame rendering that
  /// happens after any deferred work queued by the [onBeginFrame] phase.
  /// {@endtemplate}
  VoidCallback? get onDrawFrame => _onDrawFrame;
  VoidCallback? _onDrawFrame;
  Zone _onDrawFrameZone = Zone.root;
  set onDrawFrame(VoidCallback? callback) {
    _onDrawFrame = callback;
    _onDrawFrameZone = Zone.current;
  }

  /// A callback that is invoked when pointer data is available.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// See also:
  ///
  ///  * [GestureBinding], the Flutter framework class which manages pointer
  ///    events.
  PointerDataPacketCallback? get onPointerDataPacket => _onPointerDataPacket;
  PointerDataPacketCallback? _onPointerDataPacket;
  Zone _onPointerDataPacketZone = Zone.root;
  set onPointerDataPacket(PointerDataPacketCallback? callback) {
    _onPointerDataPacket = callback;
    _onPointerDataPacketZone = Zone.current;
  }

  /// A callback that is invoked to report the [FrameTiming] of recently
  /// rasterized frames.
  ///
  /// It's preferred to use [SchedulerBinding.addTimingsCallback] than to use
  /// [onReportTimings] directly because [SchedulerBinding.addTimingsCallback]
  /// allows multiple callbacks.
  ///
  /// This can be used to see if the application has missed frames (through
  /// [FrameTiming.buildDuration] and [FrameTiming.rasterDuration]), or high
  /// latencies (through [FrameTiming.totalSpan]).
  ///
  /// Unlike [Timeline], the timing information here is available in the release
  /// mode (additional to the profile and the debug mode). Hence this can be
  /// used to monitor the application's performance in the wild.
  ///
  /// {@macro dart.ui.TimingsCallback.list}
  ///
  /// If this is null, no additional work will be done. If this is not null,
  /// Flutter spends less than 0.1ms every 1 second to report the timings
  /// (measured on iPhone6S). The 0.1ms is about 0.6% of 16ms (frame budget for
  /// 60fps), or 0.01% CPU usage per second.
  TimingsCallback? get onReportTimings => _onReportTimings;
  TimingsCallback? _onReportTimings;
  Zone _onReportTimingsZone = Zone.root;
  set onReportTimings(TimingsCallback? callback) {
    if ((callback == null) != (_onReportTimings == null)) {
      _setNeedsReportTimings(callback != null);
    }
    _onReportTimings = callback;
    _onReportTimingsZone = Zone.current;
  }

  late _SetNeedsReportTimingsFunc _setNeedsReportTimings;
  void _nativeSetNeedsReportTimings(bool value)
      native 'PlatformConfiguration_setNeedsReportTimings';

  /// Sends a message to a platform-specific plugin.
  ///
  /// The `name` parameter determines which plugin receives the message. The
  /// `data` parameter contains the message payload and is typically UTF-8
  /// encoded JSON but can be arbitrary data. If the plugin replies to the
  /// message, `callback` will be called with the response.
  ///
  /// The framework invokes [callback] in the same zone in which this method was
  /// called.
  void sendPlatformMessage(String name, ByteData? data, PlatformMessageResponseCallback? callback) {
    final String? error =
        _sendPlatformMessage(name, _zonedPlatformMessageResponseCallback(callback), data);
    if (error != null) {
      throw Exception(error);
    }
  }

  String? _sendPlatformMessage(String name, PlatformMessageResponseCallback? callback, ByteData? data)
      native 'PlatformConfiguration_sendPlatformMessage';

  /// Called whenever this platform dispatcher receives a message from a
  /// platform-specific plugin.
  ///
  /// The `name` parameter determines which plugin sent the message. The `data`
  /// parameter is the payload and is typically UTF-8 encoded JSON but can be
  /// arbitrary data.
  ///
  /// Message handlers must call the function given in the `callback` parameter.
  /// If the handler does not need to respond, the handler should pass null to
  /// the callback.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  PlatformMessageCallback? get onPlatformMessage => _onPlatformMessage;
  PlatformMessageCallback? _onPlatformMessage;
  Zone _onPlatformMessageZone = Zone.root;
  set onPlatformMessage(PlatformMessageCallback? callback) {
    _onPlatformMessage = callback;
    _onPlatformMessageZone = Zone.current;
  }

  /// Called by [_dispatchPlatformMessage].
  void _respondToPlatformMessage(int responseId, ByteData? data)
      native 'PlatformConfiguration_respondToPlatformMessage';

  void _dispatchPlatformMessage(String name, ByteData? data, int responseId) {
    if (name == ChannelBuffers.kControlChannelName) {
      try {
        channelBuffers.handleMessage(data!);
      } catch (ex) {
        _printDebug('Message to "$name" caused exception $ex');
      } finally {
        _respondToPlatformMessage(responseId, null);
      }
    } else if (onPlatformMessage != null) {
      _invoke3<String, ByteData?, PlatformMessageResponseCallback>(
        onPlatformMessage,
        _onPlatformMessageZone,
        name,
        data,
            (ByteData? responseData) {
          _respondToPlatformMessage(responseId, responseData);
        },
      );
    } else {
      channelBuffers.push(name, data, (ByteData? responseData) {
        _respondToPlatformMessage(responseId, responseData);
      });
    }
  }

  /// Wraps the given [callback] in another callback that ensures that the
  /// original callback is called in the zone it was registered in.
  static PlatformMessageResponseCallback? _zonedPlatformMessageResponseCallback(
      PlatformMessageResponseCallback? callback) {
    if (callback == null) {
      return null;
    }

    // Store the zone in which the callback is being registered.
    final Zone registrationZone = Zone.current;

    return (ByteData? data) {
      registrationZone.runUnaryGuarded(callback, data);
    };
  }

  /// Set the debug name associated with this platform dispatcher's root
  /// isolate.
  ///
  /// Normally debug names are automatically generated from the Dart port, entry
  /// point, and source file. For example: `main.dart$main-1234`.
  ///
  /// This can be combined with flutter tools `--isolate-filter` flag to debug
  /// specific root isolates. For example: `flutter attach --isolate-filter=[name]`.
  /// Note that this does not rename any child isolates of the root.
  void setIsolateDebugName(String name) native 'PlatformConfiguration_setIsolateDebugName';

  /// The embedder can specify data that the isolate can request synchronously
  /// on launch. This accessor fetches that data.
  ///
  /// This data is persistent for the duration of the Flutter application and is
  /// available even after isolate restarts. Because of this lifecycle, the size
  /// of this data must be kept to a minimum.
  ///
  /// For asynchronous communication between the embedder and isolate, a
  /// platform channel may be used.
  ByteData? getPersistentIsolateData() native 'PlatformConfiguration_getPersistentIsolateData';

  /// Requests that, at the next appropriate opportunity, the [onBeginFrame] and
  /// [onDrawFrame] callbacks be invoked.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  void scheduleFrame() native 'PlatformConfiguration_scheduleFrame';

  /// Updates the application's rendering on the GPU with the newly provided
  /// [Scene]. This function must be called within the scope of the
  /// [onBeginFrame] or [onDrawFrame] callbacks being invoked.
  ///
  /// If given, draws the scene into the given `view`. If `view` is null, or no
  /// `view` is given, then the scene is drawn into the default [window].
  ///
  /// If this function is called a second time during a single
  /// [onBeginFrame]/[onDrawFrame] callback sequence or called outside the scope
  /// of those callbacks, the call will be ignored.
  ///
  /// To record graphical operations, first create a [PictureRecorder], then
  /// construct a [Canvas], passing that [PictureRecorder] to its constructor.
  /// After issuing all the graphical operations, call the
  /// [PictureRecorder.endRecording] function on the [PictureRecorder] to obtain
  /// the final [Picture] that represents the issued graphical operations.
  ///
  /// Next, create a [SceneBuilder], and add the [Picture] to it using
  /// [SceneBuilder.addPicture]. With the [SceneBuilder.build] method you can
  /// then obtain a [Scene] object, which you can display to the user via this
  /// [render] function.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  ///  * [RendererBinding], the Flutter framework class which manages layout and
  ///    painting.
  void render(Scene scene, [FlutterView? view]) native 'PlatformConfiguration_render';

  /// Additional accessibility features that may be enabled by the platform.
  AccessibilityFeatures get accessibilityFeatures => configuration.accessibilityFeatures;

  /// A callback that is invoked when the value of [accessibilityFeatures]
  /// changes.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  VoidCallback? get onAccessibilityFeaturesChanged => _onAccessibilityFeaturesChanged;
  VoidCallback? _onAccessibilityFeaturesChanged;
  Zone _onAccessibilityFeaturesChangedZone = Zone.root;
  set onAccessibilityFeaturesChanged(VoidCallback? callback) {
    _onAccessibilityFeaturesChanged = callback;
    _onAccessibilityFeaturesChangedZone = Zone.current;
  }

  void _updateAccessibilityFeatures(AccessibilityFeatures newFeatures) {
    if (newFeatures == configuration.accessibilityFeatures) {
      return;
    }
    _configuration = configuration.copyWith(accessibilityFeatures: newFeatures);
    _invoke(onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
    _invoke(onAccessibilityFeaturesChanged, _onAccessibilityFeaturesChangedZone);
  }

  /// Change the retained semantics data about this platform dispatcher.
  ///
  /// If [semanticsEnabled] is true, the user has requested that this function
  /// be called whenever the semantic content of this platform dispatcher
  /// changes.
  ///
  /// In either case, this function disposes the given update, which means the
  /// semantics update cannot be used further.
  void updateSemantics(SemanticsUpdate update) native 'PlatformConfiguration_updateSemantics';

  /// The system-reported default locale of the device.
  ///
  /// This establishes the language and formatting conventions that application
  /// should, if possible, use to render their user interface.
  ///
  /// This is the first locale selected by the user and is the user's primary
  /// locale (the locale the device UI is displayed in)
  ///
  /// This is equivalent to `locales.first` and will provide an empty non-null
  /// locale if the [locales] list has not been set or is empty.
  Locale? get locale {
    if (configuration.locales.isNotEmpty) {
      return configuration.locales.first;
    }
    return null;
  }

  /// The full system-reported supported locales of the device.
  ///
  /// This establishes the language and formatting conventions that application
  /// should, if possible, use to render their user interface.
  ///
  /// The list is ordered in order of priority, with lower-indexed locales being
  /// preferred over higher-indexed ones. The first element is the primary
  /// [locale].
  ///
  /// The [onLocaleChanged] callback is called whenever this value changes.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  List<Locale>? get locales => configuration.locales;

  void _updateLocales(List<String> locales) {
    const int stringsPerLocale = 4;
    final int numLocales = locales.length ~/ stringsPerLocale;
    final PlatformConfiguration previousConfiguration = configuration;
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
    _configuration = previousConfiguration.copyWith(locales: newLocales);
    _invoke(onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
    _invoke(onLocaleChanged, _onLocaleChangedZone);
  }

  void _updatePlatformResolvedLocale(List<String> localeData) {
    if (localeData.length != 4) {
      return;
    }
    final String countryCode = localeData[1];
    final String scriptCode = localeData[2];
    final PlatformConfiguration previousConfiguration = configuration;
    final Locale resolvedLocale = Locale.fromSubtags(
      languageCode: localeData[0],
      countryCode: countryCode.isEmpty ? null : countryCode,
      scriptCode: scriptCode.isEmpty ? null : scriptCode,
    );
    if (previousConfiguration.platformResolvedLocale == resolvedLocale) {
      return;
    }

    _configuration = previousConfiguration.copyWith(platformResolvedLocale: resolvedLocale);
    _invoke(onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
    _invoke(onLocaleChanged, _onLocaleChangedZone);
  }

  /// Performs the platform-native locale resolution.
  ///
  /// Each platform may return different results.
  ///
  /// If the platform fails to resolve a locale, then this will return null.
  ///
  /// This method returns synchronously and is a direct call to
  /// platform specific APIs without invoking method channels.
  Locale? computePlatformResolvedLocale(List<Locale> supportedLocales) {
    final List<String?> supportedLocalesData = <String?>[];
    for (Locale locale in supportedLocales) {
      supportedLocalesData.add(locale.languageCode);
      supportedLocalesData.add(locale.countryCode);
      supportedLocalesData.add(locale.scriptCode);
    }

    final List<String> result = _computePlatformResolvedLocale(supportedLocalesData);

    if (result.isNotEmpty) {
      return Locale.fromSubtags(
        languageCode: result[0],
        countryCode: result[1] == '' ? null : result[1],
        scriptCode: result[2] == '' ? null : result[2]);
    }
    return null;
  }
  List<String> _computePlatformResolvedLocale(List<String?> supportedLocalesData) native 'Window_computePlatformResolvedLocale';


  /// A callback that is invoked whenever [locale] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onLocaleChanged => _onLocaleChanged;
  VoidCallback? _onLocaleChanged;
  Zone _onLocaleChangedZone = Zone.root; // ignore: unused_field
  set onLocaleChanged(VoidCallback? callback) {
    _onLocaleChanged = callback;
    _onLocaleChangedZone = Zone.current;
  }

  /// The lifecycle state immediately after dart isolate initialization.
  ///
  /// This property will not be updated as the lifecycle changes.
  ///
  /// It is used to initialize [SchedulerBinding.lifecycleState] at startup with
  /// any buffered lifecycle state events.
  String get initialLifecycleState {
    _initialLifecycleStateAccessed = true;
    return _initialLifecycleState;
  }
  late String _initialLifecycleState;

  void _updateLifecycleState(String state) {
    // We do not update the state if the state has already been used to initialize
    // the lifecycleState.
    if (!_initialLifecycleStateAccessed)
      _initialLifecycleState = state;
  }

  /// Tracks if the initial state has been accessed. Once accessed, we will stop
  /// updating the [initialLifecycleState], as it is not the preferred way to
  /// access the state.
  bool _initialLifecycleStateAccessed = false;

  /// The system-reported text scale.
  ///
  /// This establishes the text scaling factor to use when rendering text,
  /// according to the user's platform preferences.
  ///
  /// The [onTextScaleFactorChanged] callback is called whenever this value
  /// changes.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  double get textScaleFactor => configuration.textScaleFactor;

  /// The setting indicating whether time should always be shown in the 24-hour
  /// format.
  ///
  /// This option is used by [showTimePicker].
  bool get alwaysUse24HourFormat => configuration.alwaysUse24HourFormat;

  /// A callback that is invoked whenever [textScaleFactor] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onTextScaleFactorChanged => _onTextScaleFactorChanged;
  VoidCallback? _onTextScaleFactorChanged;
  Zone _onTextScaleFactorChangedZone = Zone.root;
  set onTextScaleFactorChanged(VoidCallback? callback) {
    _onTextScaleFactorChanged = callback;
    _onTextScaleFactorChangedZone = Zone.current;
  }

  /// The setting indicating the current brightness mode of the host platform.
  /// If the platform has no preference, [platformBrightness] defaults to
  /// [Brightness.light].
  Brightness get platformBrightness => configuration.platformBrightness;

  /// A callback that is invoked whenever [platformBrightness] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onPlatformBrightnessChanged => _onPlatformBrightnessChanged;
  VoidCallback? _onPlatformBrightnessChanged;
  Zone _onPlatformBrightnessChangedZone = Zone.root;
  set onPlatformBrightnessChanged(VoidCallback? callback) {
    _onPlatformBrightnessChanged = callback;
    _onPlatformBrightnessChangedZone = Zone.current;
  }

  void _updateUserSettingsData(Map<String, dynamic> data) {
    final double textScaleFactor = (data['textScaleFactor'] as num).toDouble();
    final bool alwaysUse24HourFormat = data['alwaysUse24HourFormat'] as bool;
    final Brightness platformBrightness =
    data['platformBrightness'] as String == 'dark' ? Brightness.dark : Brightness.light;
    final PlatformConfiguration previousConfiguration = configuration;
    final bool platformBrightnessChanged =
        previousConfiguration.platformBrightness != platformBrightness;
    final bool textScaleFactorChanged = previousConfiguration.textScaleFactor != textScaleFactor;
    final bool alwaysUse24HourFormatChanged =
        previousConfiguration.alwaysUse24HourFormat != alwaysUse24HourFormat;
    if (!platformBrightnessChanged && !textScaleFactorChanged && !alwaysUse24HourFormatChanged) {
      return;
    }
    _configuration = previousConfiguration.copyWith(
      textScaleFactor: textScaleFactor,
      alwaysUse24HourFormat: alwaysUse24HourFormat,
      platformBrightness: platformBrightness,
    );
    _invoke(onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
    if (textScaleFactorChanged) {
      _invoke(onTextScaleFactorChanged, _onTextScaleFactorChangedZone);
    }
    if (platformBrightnessChanged) {
      _invoke(onPlatformBrightnessChanged, _onPlatformBrightnessChangedZone);
    }
  }

  /// Whether the user has requested that [updateSemantics] be called when the
  /// semantic contents of a view changes.
  ///
  /// The [onSemanticsEnabledChanged] callback is called whenever this value
  /// changes.
  bool get semanticsEnabled => configuration.semanticsEnabled;

  /// A callback that is invoked when the value of [semanticsEnabled] changes.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  VoidCallback? get onSemanticsEnabledChanged => _onSemanticsEnabledChanged;
  VoidCallback? _onSemanticsEnabledChanged;
  Zone _onSemanticsEnabledChangedZone = Zone.root;
  set onSemanticsEnabledChanged(VoidCallback? callback) {
    _onSemanticsEnabledChanged = callback;
    _onSemanticsEnabledChangedZone = Zone.current;
  }

  void _updateSemanticsEnabled(bool enabled) {
    if (configuration.semanticsEnabled == enabled) {
      return;
    }
    _configuration = configuration.copyWith(semanticsEnabled: enabled);
    _invoke(onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
    _invoke(onSemanticsEnabledChanged, _onSemanticsEnabledChangedZone);
  }

  /// A callback that is invoked whenever the user requests an action to be
  /// performed.
  ///
  /// This callback is used when the user expresses the action they wish to
  /// perform based on the semantics supplied by [updateSemantics].
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  SemanticsActionCallback? get onSemanticsAction => _onSemanticsAction;
  SemanticsActionCallback? _onSemanticsAction;
  Zone _onSemanticsActionZone = Zone.root;
  set onSemanticsAction(SemanticsActionCallback? callback) {
    _onSemanticsAction = callback;
    _onSemanticsActionZone = Zone.current;
  }

  /// The route or path that the embedder requested when the application was
  /// launched.
  ///
  /// This will be the string "`/`" if no particular route was requested.
  ///
  /// ## Android
  ///
  /// On Android, calling
  /// [`FlutterView.setInitialRoute`](/javadoc/io/flutter/view/FlutterView.html#setInitialRoute-java.lang.String-)
  /// will set this value. The value must be set sufficiently early, i.e. before
  /// the [runApp] call is executed in Dart, for this to have any effect on the
  /// framework. The `createFlutterView` method in your `FlutterActivity`
  /// subclass is a suitable time to set the value. The application's
  /// `AndroidManifest.xml` file must also be updated to have a suitable
  /// [`<intent-filter>`](https://developer.android.com/guide/topics/manifest/intent-filter-element.html).
  ///
  /// ## iOS
  ///
  /// On iOS, calling
  /// [`FlutterViewController.setInitialRoute`](/objcdoc/Classes/FlutterViewController.html#/c:objc%28cs%29FlutterViewController%28im%29setInitialRoute:)
  /// will set this value. The value must be set sufficiently early, i.e. before
  /// the [runApp] call is executed in Dart, for this to have any effect on the
  /// framework. The `application:didFinishLaunchingWithOptions:` method is a
  /// suitable time to set this value.
  ///
  /// See also:
  ///
  ///  * [Navigator], a widget that handles routing.
  ///  * [SystemChannels.navigation], which handles subsequent navigation
  ///    requests from the embedder.
  String get initialRouteName => _initialRouteName();
  String _initialRouteName() native 'PlatformConfiguration_initialRouteName';
}

/// The immutable configuration details of the platform that the application is
/// running on.
///
/// This is used by the [PlatformDispatcher] to hold the platform's
/// configuration details so that they can be atomically updated by the engine.
///
/// See also:
///
///  * [PlatformDispatcher.configuration], which provides access this
///    information.
class PlatformConfiguration {
  /// Const constructor for [PlatformConfiguration].
  const PlatformConfiguration({
    this.accessibilityFeatures = const AccessibilityFeatures._(0),
    this.alwaysUse24HourFormat = false,
    this.semanticsEnabled = false,
    this.platformBrightness = Brightness.light,
    this.textScaleFactor = 1.0,
    this.locales = const <Locale>[],
    this.platformResolvedLocale,
    this.initialRouteName,
  });

  /// Copy a [PlatformConfiguration] with some fields replaced.
  PlatformConfiguration copyWith({
    AccessibilityFeatures? accessibilityFeatures,
    bool? alwaysUse24HourFormat,
    bool? semanticsEnabled,
    Brightness? platformBrightness,
    double? textScaleFactor,
    List<Locale>? locales,
    Locale? platformResolvedLocale,
    String? initialRouteName,
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

  /// Additional accessibility features that may be enabled by the platform.
  final AccessibilityFeatures accessibilityFeatures;

  /// The setting indicating whether time should always be shown in the 24-hour
  /// format.
  final bool alwaysUse24HourFormat;

  /// Whether the user has requested that [updateSemantics] be called when the
  /// semantic contents of a view changes.
  final bool semanticsEnabled;

  /// The setting indicating the current brightness mode of the host platform.
  /// If the platform has no preference, [platformBrightness] defaults to
  /// [Brightness.light].
  final Brightness platformBrightness;

  /// The system-reported text scale.
  final double textScaleFactor;

  /// The full system-reported supported locales of the device.
  final List<Locale> locales;

  /// The system-reported default locale of the device.
  final Locale? platformResolvedLocale;

  /// The route or path that the embedder requested when the application was
  /// launched.
  final String? initialRouteName;
}

/// The immutable configuration information for a screen.
///
/// This is used by the [PlatformDispatcher] to hold a screen's configuration
/// details so that they can be atomically updated by the engine.
///
/// See also:
///
///  * [Screen], which represents a particular screen, including this information.
///  * [PlatformDispatcher.screens], which provides access to the screens on
///    this device.
class ScreenConfiguration {
  /// Const constructor for [ScreenConfiguration] information.
  const ScreenConfiguration({
    this.screenName = '',
    this.geometry = Rect.zero,
    this.devicePixelRatio = 1.0,
    this.viewInsets = WindowPadding.zero,
    this.viewPadding = WindowPadding.zero,
    this.systemGestureInsets = WindowPadding.zero,
    this.padding = WindowPadding.zero,
  });

  /// Makes a new copy of this [ScreenConfiguration] with some attributes
  /// replaced.
  ScreenConfiguration copyWith({
    String? screenName,
    Rect? geometry,
    double? devicePixelRatio,
    WindowPadding? viewInsets,
    WindowPadding? viewPadding,
    WindowPadding? systemGestureInsets,
    WindowPadding? padding,
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

  /// Platform-provided name for screen.
  final String screenName;

  /// Screen rect in Flutter logical pixels
  final Rect geometry;

  /// Device pixel ratio in device pixels to logical pixels.
  final double devicePixelRatio;

  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but over which the operating
  /// system will likely place system UI, such as the keyboard or system menus,
  /// that fully obscures any content.
  final WindowPadding viewInsets;

  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but which may be partially
  /// obscured by system UI (such as the system notification area), or physical
  /// intrusions in the display (e.g. overscan regions on television screens or
  /// phone sensor housings).
  final WindowPadding viewPadding;

  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but where the operating system
  /// will consume input gestures for the sake of system navigation.
  final WindowPadding systemGestureInsets;

  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but which may be partially
  /// obscured by system UI (such as the system notification area), or physical
  /// intrusions in the display (e.g. overscan regions on television screens or
  /// phone sensor housings).
  final WindowPadding padding;

  @override
  String toString() {
    return '$runtimeType[screenName: $screenName, geometry: $geometry, devicePixelRatio: $devicePixelRatio]';
  }
}

/// The immutable configuration information for a view.
///
/// This is used by the [PlatformDispatcher] to hold a view's configuration
/// details so that they can be atomically updated by the engine.
///
/// See also:
///
///  * [FlutterView], which represents a particular view, including this information.
///  * [PlatformDispatcher.views], which provides access to the views on
///    this device.
class ViewConfiguration {
  /// A const constructor for an immutable [ViewConfiguration].
  const ViewConfiguration(
    this.screen,
  { this.window,
    this.geometry = Rect.zero,
    this.depth = double.maxFinite,
    this.visible = false,
    this.viewInsets = WindowPadding.zero,
    this.viewPadding = WindowPadding.zero,
    this.systemGestureInsets = WindowPadding.zero,
    this.padding = WindowPadding.zero,
  });

  /// Copy this configuration with some fields replaced.
  ViewConfiguration copyWith({
    Screen? screen,
    FlutterWindow? window,
    Rect? geometry,
    double? depth,
    bool? visible,
    WindowPadding? viewInsets,
    WindowPadding? viewPadding,
    WindowPadding? systemGestureInsets,
    WindowPadding? padding,
  }) {
    return ViewConfiguration(
      screen ?? this.screen,
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

  /// The screen that this view should appear on.
  ///
  /// This is the screen that the upper left corner of the view appears on.
  final Screen screen;

  /// The top level view into which the view is placed and its geometry is
  /// relative to.
  ///
  /// If null, then this configuration represents a top level view itself.
  final FlutterWindow? window;

  /// The geometry requested for the view on the [screen] or within its parent
  /// window, in logical pixels.
  ///
  /// This uses the device pixel ratio of the [screen].
  final Rect geometry;

  /// The depth that is the maximum elevation that the view allows.
  ///
  /// Physical layers drawn at or above this elevation will have their elevation
  /// clamped to this value. This can happen if the physical layer itself has an
  /// elevation larger than available depth, or if some ancestor of the layer
  /// causes it to have a cumulative elevation that is larger than the available
  /// depth.
  ///
  /// The default value is [double.maxFinite], which is used for platforms that
  /// do not specify a maximum elevation. This property is currently only
  /// expected to be set to a non-default value on Fuchsia.
  final double depth;

  /// Whether or not the view is currently visible on the screen.
  final bool visible;

  /// The view insets, as it intersects with [Screen.viewInsets] for the screen
  /// it is on.
  ///
  /// For instance, if the view doesn't overlap the
  /// [ScreenConfiguration.viewInsets] area, [viewInsets] will be
  /// [WindowPadding.zero].
  ///
  /// The number of physical pixels on each side of this view rectangle into
  /// which the application can draw, but over which the operating system will
  /// likely place system UI, such as the keyboard or system menus, that fully
  /// obscures any content.
  final WindowPadding viewInsets;

  /// The view insets, as it intersects with [ScreenConfiguration.viewPadding]
  /// for the screen it is on.
  ///
  /// For instance, if the view doesn't overlap the
  /// [ScreenConfiguration.viewPadding] area, [viewPadding] will be
  /// [WindowPadding.zero].
  ///
  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but which may be partially
  /// obscured by system UI (such as the system notification area), or physical
  /// intrusions in the display (e.g. overscan regions on television screens or
  /// phone sensor housings).
  final WindowPadding viewPadding;

  /// The view insets, as it intersects with
  /// [ScreenConfiguration.systemGestureInsets] for the screen it is on.
  ///
  /// For instance, if the view doesn't overlap the
  /// [ScreenConfiguration.systemGestureInsets] area, [systemGestureInsets] will
  /// be [WindowPadding.zero].
  ///
  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but where the operating system
  /// will consume input gestures for the sake of system navigation.
  final WindowPadding systemGestureInsets;

  /// The view insets, as it intersects with [ScreenConfiguration.padding] for
  /// the screen it is on.
  ///
  /// For instance, if the view doesn't overlap the
  /// [ScreenConfiguration.padding] area, [padding] will be
  /// [WindowPadding.zero].
  ///
  /// The number of physical pixels on each side of this screen rectangle into
  /// which the application can place a view, but which may be partially
  /// obscured by system UI (such as the system notification area), or physical
  /// intrusions in the display (e.g. overscan regions on television screens or
  /// phone sensor housings).
  final WindowPadding padding;

  @override
  String toString() {
    return '$runtimeType[screen: $screen, window: $window, geometry: $geometry, depth: $depth]';
  }
}
