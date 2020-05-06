// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9
part of dart.ui;

/// Signature of callbacks that have no arguments and return no data.
typedef VoidCallback = void Function();

/// Signature for [FlutterWindow.onBeginFrame].
typedef FrameCallback = void Function(Duration duration);

/// Signature for [FlutterWindow.onReportTimings].
///
/// {@template dart.ui.TimingsCallback.list}
/// The callback takes a list of [FrameTiming] because it may not be
/// immediately triggered after each frame. Instead, Flutter tries to batch
/// frames together and send all their timings at once to decrease the
/// overhead (as this is available in the release mode). The list is sorted in
/// ascending order of time (earliest frame first). The timing of any frame
/// will be sent within about 1 second (100ms if in the profile/debug mode)
/// even if there are no later frames to batch. The timing of the first frame
/// will be sent immediately without batching.
/// {@endtemplate}
typedef TimingsCallback = void Function(List<FrameTiming> timings);

/// Signature for [FlutterWindow.onPointerDataPacket].
typedef PointerDataPacketCallback = void Function(PointerDataPacket packet);

/// Signature for [FlutterWindow.onSemanticsAction].
typedef SemanticsActionCallback = void Function(int id, SemanticsAction action, ByteData? args);

/// Signature for responses to platform messages.
///
/// Used as a parameter to [FlutterWindow.sendPlatformMessage] and
/// [FlutterWindow.onPlatformMessage].
typedef PlatformMessageResponseCallback = void Function(ByteData? data);

/// Signature for [FlutterWindow.onPlatformMessage].
typedef PlatformMessageCallback = void Function(String name, ByteData? data, PlatformMessageResponseCallback? callback);

// Signature for _setNeedsReportTimings.
typedef _SetNeedsReportTimingsFunc = void Function(bool value);

/// Various important time points in the lifetime of a frame.
///
/// [FrameTiming] records a timestamp of each phase for performance analysis.
enum FramePhase {
  /// When the UI thread starts building a frame.
  ///
  /// See also [FrameTiming.buildDuration].
  buildStart,

  /// When the UI thread finishes building a frame.
  ///
  /// See also [FrameTiming.buildDuration].
  buildFinish,

  /// When the raster thread starts rasterizing a frame.
  ///
  /// See also [FrameTiming.rasterDuration].
  rasterStart,

  /// When the raster thread finishes rasterizing a frame.
  ///
  /// See also [FrameTiming.rasterDuration].
  rasterFinish,
}

/// Time-related performance metrics of a frame.
///
/// If you're using the whole Flutter framework, please use
/// [SchedulerBinding.addTimingsCallback] to get this. It's preferred over using
/// [FlutterWindow.onReportTimings] directly because
/// [SchedulerBinding.addTimingsCallback] allows multiple callbacks. If
/// [SchedulerBinding] is unavailable, then see [FlutterWindow.onReportTimings]
/// for how to get this.
///
/// The metrics in debug mode (`flutter run` without any flags) may be very
/// different from those in profile and release modes due to the debug overhead.
/// Therefore it's recommended to only monitor and analyze performance metrics
/// in profile and release modes.
class FrameTiming {
  /// Construct [FrameTiming] with raw timestamps in microseconds.
  ///
  /// List [timestamps] must have the same number of elements as
  /// [FramePhase.values].
  ///
  /// This constructor is usually only called by the Flutter engine, or a test.
  /// To get the [FrameTiming] of your app, see [FlutterWindow.onReportTimings].
  FrameTiming(List<int> timestamps)
      : assert(timestamps.length == FramePhase.values.length),
        _timestamps = timestamps;

  /// This is a raw timestamp in microseconds from some epoch. The epoch in all
  /// [FrameTiming] is the same, but it may not match [DateTime]'s epoch.
  int timestampInMicroseconds(FramePhase phase) => _timestamps[phase.index];

  Duration _rawDuration(FramePhase phase) => Duration(microseconds: _timestamps[phase.index]);

  /// The duration to build the frame on the UI thread.
  ///
  /// The build starts approximately when [FlutterWindow.onBeginFrame] is called. The
  /// [Duration] in the [FlutterWindow.onBeginFrame] callback is exactly the
  /// `Duration(microseconds: timestampInMicroseconds(FramePhase.buildStart))`.
  ///
  /// The build finishes when [FlutterWindow.render] is called.
  ///
  /// {@template dart.ui.FrameTiming.fps_smoothness_milliseconds}
  /// To ensure smooth animations of X fps, this should not exceed 1000/X
  /// milliseconds.
  /// {@endtemplate}
  /// {@template dart.ui.FrameTiming.fps_milliseconds}
  /// That's about 16ms for 60fps, and 8ms for 120fps.
  /// {@endtemplate}
  Duration get buildDuration => _rawDuration(FramePhase.buildFinish) - _rawDuration(FramePhase.buildStart);

  /// The duration to rasterize the frame on the raster thread.
  ///
  /// {@macro dart.ui.FrameTiming.fps_smoothness_milliseconds}
  /// {@macro dart.ui.FrameTiming.fps_milliseconds}
  Duration get rasterDuration => _rawDuration(FramePhase.rasterFinish) - _rawDuration(FramePhase.rasterStart);

  /// The timespan between build start and raster finish.
  ///
  /// To achieve the lowest latency on an X fps display, this should not exceed
  /// 1000/X milliseconds.
  /// {@macro dart.ui.FrameTiming.fps_milliseconds}
  ///
  /// See also [buildDuration] and [rasterDuration].
  Duration get totalSpan => _rawDuration(FramePhase.rasterFinish) - _rawDuration(FramePhase.buildStart);

  final List<int> _timestamps;  // in microseconds

  String _formatMS(Duration duration) => '${duration.inMicroseconds * 0.001}ms';

  @override
  String toString() {
    return '$runtimeType(buildDuration: ${_formatMS(buildDuration)}, rasterDuration: ${_formatMS(rasterDuration)}, totalSpan: ${_formatMS(totalSpan)})';
  }
}

/// States that an application can be in.
///
/// The values below describe notifications from the operating system.
/// Applications should not expect to always receive all possible
/// notifications. For example, if the users pulls out the battery from the
/// device, no notification will be sent before the application is suddenly
/// terminated, along with the rest of the operating system.
///
/// See also:
///
///  * [WidgetsBindingObserver], for a mechanism to observe the lifecycle state
///    from the widgets layer.
enum AppLifecycleState {
  /// The application is visible and responding to user input.
  resumed,

  /// The application is in an inactive state and is not receiving user input.
  ///
  /// On iOS, this state corresponds to an app or the Flutter host view running
  /// in the foreground inactive state. Apps transition to this state when in
  /// a phone call, responding to a TouchID request, when entering the app
  /// switcher or the control center, or when the UIViewController hosting the
  /// Flutter app is transitioning.
  ///
  /// On Android, this corresponds to an app or the Flutter host view running
  /// in the foreground inactive state.  Apps transition to this state when
  /// another activity is focused, such as a split-screen app, a phone call,
  /// a picture-in-picture app, a system dialog, or another window.
  ///
  /// Apps in this state should assume that they may be [paused] at any time.
  inactive,

  /// The application is not currently visible to the user, not responding to
  /// user input, and running in the background.
  ///
  /// When the application is in this state, the engine will not call the
  /// [FlutterWindow.onBeginFrame] and [FlutterWindow.onDrawFrame] callbacks.
  paused,

  /// The application is still hosted on a flutter engine but is detached from
  /// any host views.
  ///
  /// When the application is in this state, the engine is running without
  /// a view. It can either be in the progress of attaching a view when engine
  /// was first initializes, or after the view being destroyed due to a Navigator
  /// pop.
  detached,
}

/// A representation of distances for each of the four edges of a rectangle,
/// used to encode the view insets and padding that applications should place
/// around their user interface, as exposed by [FlutterWindow.viewInsets] and
/// [FlutterWindow.padding]. View insets and padding are preferably read via
/// [MediaQuery.of].
///
/// For a generic class that represents distances around a rectangle, see the
/// [EdgeInsets] class.
///
/// See also:
///
///  * [WidgetsBindingObserver], for a widgets layer mechanism to receive
///    notifications when the padding changes.
///  * [MediaQuery.of], for the preferred mechanism for accessing these values.
///  * [Scaffold], which automatically applies the padding in material design
///    applications.
class WindowPadding {
  const WindowPadding._({ required this.left, required this.top, required this.right, required this.bottom });

  /// The distance from the left edge to the first unpadded pixel, in physical pixels.
  final double left;

  /// The distance from the top edge to the first unpadded pixel, in physical pixels.
  final double top;

  /// The distance from the right edge to the first unpadded pixel, in physical pixels.
  final double right;

  /// The distance from the bottom edge to the first unpadded pixel, in physical pixels.
  final double bottom;

  /// A window padding that has zeros for each edge.
  static const WindowPadding zero = WindowPadding._(left: 0.0, top: 0.0, right: 0.0, bottom: 0.0);

  @override
  String toString() {
    return 'WindowPadding(left: $left, top: $top, right: $right, bottom: $bottom)';
  }
}

/// An identifier used to select a user's language and formatting preferences.
///
/// This represents a [Unicode Language
/// Identifier](https://www.unicode.org/reports/tr35/#Unicode_language_identifier)
/// (i.e. without Locale extensions), except variants are not supported.
///
/// Locales are canonicalized according to the "preferred value" entries in the
/// [IANA Language Subtag
/// Registry](https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry).
/// For example, `const Locale('he')` and `const Locale('iw')` are equal and
/// both have the [languageCode] `he`, because `iw` is a deprecated language
/// subtag that was replaced by the subtag `he`.
///
/// See also:
///
///  * [FlutterWindow.locale], which specifies the system's currently selected
///    [Locale].
class Locale {
  /// Creates a new Locale object. The first argument is the
  /// primary language subtag, the second is the region (also
  /// referred to as 'country') subtag.
  ///
  /// For example:
  ///
  /// ```dart
  /// const Locale swissFrench = Locale('fr', 'CH');
  /// const Locale canadianFrench = Locale('fr', 'CA');
  /// ```
  ///
  /// The primary language subtag must not be null. The region subtag is
  /// optional. When there is no region/country subtag, the parameter should
  /// be omitted or passed `null` instead of an empty-string.
  ///
  /// The subtag values are _case sensitive_ and must be one of the valid
  /// subtags according to CLDR supplemental data:
  /// [language](http://unicode.org/cldr/latest/common/validity/language.xml),
  /// [region](http://unicode.org/cldr/latest/common/validity/region.xml). The
  /// primary language subtag must be at least two and at most eight lowercase
  /// letters, but not four letters. The region region subtag must be two
  /// uppercase letters or three digits. See the [Unicode Language
  /// Identifier](https://www.unicode.org/reports/tr35/#Unicode_language_identifier)
  /// specification.
  ///
  /// Validity is not checked by default, but some methods may throw away
  /// invalid data.
  ///
  /// See also:
  ///
  ///  * [Locale.fromSubtags], which also allows a [scriptCode] to be
  ///    specified.
  const Locale(
    this._languageCode, [
    this._countryCode,
  ]) : assert(_languageCode != null), // ignore: unnecessary_null_comparison
       assert(_languageCode != ''),
       scriptCode = null;

  /// Creates a new Locale object.
  ///
  /// The keyword arguments specify the subtags of the Locale.
  ///
  /// The subtag values are _case sensitive_ and must be valid subtags according
  /// to CLDR supplemental data:
  /// [language](http://unicode.org/cldr/latest/common/validity/language.xml),
  /// [script](http://unicode.org/cldr/latest/common/validity/script.xml) and
  /// [region](http://unicode.org/cldr/latest/common/validity/region.xml) for
  /// each of languageCode, scriptCode and countryCode respectively.
  ///
  /// The [countryCode] subtag is optional. When there is no country subtag,
  /// the parameter should be omitted or passed `null` instead of an empty-string.
  ///
  /// Validity is not checked by default, but some methods may throw away
  /// invalid data.
  const Locale.fromSubtags({
    String languageCode = 'und',
    this.scriptCode,
    String? countryCode,
  }) : assert(languageCode != null), // ignore: unnecessary_null_comparison
       assert(languageCode != ''),
       _languageCode = languageCode,
       assert(scriptCode != ''),
       assert(countryCode != ''),
       _countryCode = countryCode;

  /// The primary language subtag for the locale.
  ///
  /// This must not be null. It may be 'und', representing 'undefined'.
  ///
  /// This is expected to be string registered in the [IANA Language Subtag
  /// Registry](https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry)
  /// with the type "language". The string specified must match the case of the
  /// string in the registry.
  ///
  /// Language subtags that are deprecated in the registry and have a preferred
  /// code are changed to their preferred code. For example, `const
  /// Locale('he')` and `const Locale('iw')` are equal, and both have the
  /// [languageCode] `he`, because `iw` is a deprecated language subtag that was
  /// replaced by the subtag `he`.
  ///
  /// This must be a valid Unicode Language subtag as listed in [Unicode CLDR
  /// supplemental
  /// data](http://unicode.org/cldr/latest/common/validity/language.xml).
  ///
  /// See also:
  ///
  ///  * [Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  String get languageCode => _deprecatedLanguageSubtagMap[_languageCode] ?? _languageCode;
  final String _languageCode;

  // This map is generated by //flutter/tools/gen_locale.dart
  // Mappings generated for language subtag registry as of 2019-02-27.
  static const Map<String, String> _deprecatedLanguageSubtagMap = <String, String>{
    'in': 'id', // Indonesian; deprecated 1989-01-01
    'iw': 'he', // Hebrew; deprecated 1989-01-01
    'ji': 'yi', // Yiddish; deprecated 1989-01-01
    'jw': 'jv', // Javanese; deprecated 2001-08-13
    'mo': 'ro', // Moldavian, Moldovan; deprecated 2008-11-22
    'aam': 'aas', // Aramanik; deprecated 2015-02-12
    'adp': 'dz', // Adap; deprecated 2015-02-12
    'aue': 'ktz', // ǂKxʼauǁʼein; deprecated 2015-02-12
    'ayx': 'nun', // Ayi (China); deprecated 2011-08-16
    'bgm': 'bcg', // Baga Mboteni; deprecated 2016-05-30
    'bjd': 'drl', // Bandjigali; deprecated 2012-08-12
    'ccq': 'rki', // Chaungtha; deprecated 2012-08-12
    'cjr': 'mom', // Chorotega; deprecated 2010-03-11
    'cka': 'cmr', // Khumi Awa Chin; deprecated 2012-08-12
    'cmk': 'xch', // Chimakum; deprecated 2010-03-11
    'coy': 'pij', // Coyaima; deprecated 2016-05-30
    'cqu': 'quh', // Chilean Quechua; deprecated 2016-05-30
    'drh': 'khk', // Darkhat; deprecated 2010-03-11
    'drw': 'prs', // Darwazi; deprecated 2010-03-11
    'gav': 'dev', // Gabutamon; deprecated 2010-03-11
    'gfx': 'vaj', // Mangetti Dune ǃXung; deprecated 2015-02-12
    'ggn': 'gvr', // Eastern Gurung; deprecated 2016-05-30
    'gti': 'nyc', // Gbati-ri; deprecated 2015-02-12
    'guv': 'duz', // Gey; deprecated 2016-05-30
    'hrr': 'jal', // Horuru; deprecated 2012-08-12
    'ibi': 'opa', // Ibilo; deprecated 2012-08-12
    'ilw': 'gal', // Talur; deprecated 2013-09-10
    'jeg': 'oyb', // Jeng; deprecated 2017-02-23
    'kgc': 'tdf', // Kasseng; deprecated 2016-05-30
    'kgh': 'kml', // Upper Tanudan Kalinga; deprecated 2012-08-12
    'koj': 'kwv', // Sara Dunjo; deprecated 2015-02-12
    'krm': 'bmf', // Krim; deprecated 2017-02-23
    'ktr': 'dtp', // Kota Marudu Tinagas; deprecated 2016-05-30
    'kvs': 'gdj', // Kunggara; deprecated 2016-05-30
    'kwq': 'yam', // Kwak; deprecated 2015-02-12
    'kxe': 'tvd', // Kakihum; deprecated 2015-02-12
    'kzj': 'dtp', // Coastal Kadazan; deprecated 2016-05-30
    'kzt': 'dtp', // Tambunan Dusun; deprecated 2016-05-30
    'lii': 'raq', // Lingkhim; deprecated 2015-02-12
    'lmm': 'rmx', // Lamam; deprecated 2014-02-28
    'meg': 'cir', // Mea; deprecated 2013-09-10
    'mst': 'mry', // Cataelano Mandaya; deprecated 2010-03-11
    'mwj': 'vaj', // Maligo; deprecated 2015-02-12
    'myt': 'mry', // Sangab Mandaya; deprecated 2010-03-11
    'nad': 'xny', // Nijadali; deprecated 2016-05-30
    'ncp': 'kdz', // Ndaktup; deprecated 2018-03-08
    'nnx': 'ngv', // Ngong; deprecated 2015-02-12
    'nts': 'pij', // Natagaimas; deprecated 2016-05-30
    'oun': 'vaj', // ǃOǃung; deprecated 2015-02-12
    'pcr': 'adx', // Panang; deprecated 2013-09-10
    'pmc': 'huw', // Palumata; deprecated 2016-05-30
    'pmu': 'phr', // Mirpur Panjabi; deprecated 2015-02-12
    'ppa': 'bfy', // Pao; deprecated 2016-05-30
    'ppr': 'lcq', // Piru; deprecated 2013-09-10
    'pry': 'prt', // Pray 3; deprecated 2016-05-30
    'puz': 'pub', // Purum Naga; deprecated 2014-02-28
    'sca': 'hle', // Sansu; deprecated 2012-08-12
    'skk': 'oyb', // Sok; deprecated 2017-02-23
    'tdu': 'dtp', // Tempasuk Dusun; deprecated 2016-05-30
    'thc': 'tpo', // Tai Hang Tong; deprecated 2016-05-30
    'thx': 'oyb', // The; deprecated 2015-02-12
    'tie': 'ras', // Tingal; deprecated 2011-08-16
    'tkk': 'twm', // Takpa; deprecated 2011-08-16
    'tlw': 'weo', // South Wemale; deprecated 2012-08-12
    'tmp': 'tyj', // Tai Mène; deprecated 2016-05-30
    'tne': 'kak', // Tinoc Kallahan; deprecated 2016-05-30
    'tnf': 'prs', // Tangshewi; deprecated 2010-03-11
    'tsf': 'taj', // Southwestern Tamang; deprecated 2015-02-12
    'uok': 'ema', // Uokha; deprecated 2015-02-12
    'xba': 'cax', // Kamba (Brazil); deprecated 2016-05-30
    'xia': 'acn', // Xiandao; deprecated 2013-09-10
    'xkh': 'waw', // Karahawyana; deprecated 2016-05-30
    'xsj': 'suj', // Subi; deprecated 2015-02-12
    'ybd': 'rki', // Yangbye; deprecated 2012-08-12
    'yma': 'lrr', // Yamphe; deprecated 2012-08-12
    'ymt': 'mtm', // Mator-Taygi-Karagas; deprecated 2015-02-12
    'yos': 'zom', // Yos; deprecated 2013-09-10
    'yuu': 'yug', // Yugh; deprecated 2014-02-28
  };

  /// The script subtag for the locale.
  ///
  /// This may be null, indicating that there is no specified script subtag.
  ///
  /// This must be a valid Unicode Language Identifier script subtag as listed
  /// in [Unicode CLDR supplemental
  /// data](http://unicode.org/cldr/latest/common/validity/script.xml).
  ///
  /// See also:
  ///
  ///  * [Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  final String? scriptCode;

  /// The region subtag for the locale.
  ///
  /// This may be null, indicating that there is no specified region subtag.
  ///
  /// This is expected to be string registered in the [IANA Language Subtag
  /// Registry](https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry)
  /// with the type "region". The string specified must match the case of the
  /// string in the registry.
  ///
  /// Region subtags that are deprecated in the registry and have a preferred
  /// code are changed to their preferred code. For example, `const Locale('de',
  /// 'DE')` and `const Locale('de', 'DD')` are equal, and both have the
  /// [countryCode] `DE`, because `DD` is a deprecated language subtag that was
  /// replaced by the subtag `DE`.
  ///
  /// See also:
  ///
  ///  * [Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  String? get countryCode => _deprecatedRegionSubtagMap[_countryCode] ?? _countryCode;
  final String? _countryCode;

  // This map is generated by //flutter/tools/gen_locale.dart
  // Mappings generated for language subtag registry as of 2019-02-27.
  static const Map<String, String> _deprecatedRegionSubtagMap = <String, String>{
    'BU': 'MM', // Burma; deprecated 1989-12-05
    'DD': 'DE', // German Democratic Republic; deprecated 1990-10-30
    'FX': 'FR', // Metropolitan France; deprecated 1997-07-14
    'TP': 'TL', // East Timor; deprecated 2002-05-20
    'YD': 'YE', // Democratic Yemen; deprecated 1990-08-14
    'ZR': 'CD', // Zaire; deprecated 1997-07-14
  };

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other is! Locale) {
      return false;
    }
    final String? countryCode = _countryCode;
    final String? otherCountryCode = other.countryCode;
    return other.languageCode == languageCode
        && other.scriptCode == scriptCode // scriptCode cannot be ''
        && (other.countryCode == countryCode // Treat '' as equal to null.
            || otherCountryCode != null && otherCountryCode.isEmpty && countryCode == null
            || countryCode != null && countryCode.isEmpty && other.countryCode == null);
  }

  @override
  int get hashCode => hashValues(languageCode, scriptCode, countryCode == '' ? null : countryCode);

  static Locale? _cachedLocale;
  static String? _cachedLocaleString;

  /// Returns a string representing the locale.
  ///
  /// This identifier happens to be a valid Unicode Locale Identifier using
  /// underscores as separator, however it is intended to be used for debugging
  /// purposes only. For parsable results, use [toLanguageTag] instead.
  @keepToString
  @override
  String toString() {
    if (!identical(_cachedLocale, this)) {
      _cachedLocale = this;
      _cachedLocaleString = _rawToString('_');
    }
    return _cachedLocaleString!;
  }

  /// Returns a syntactically valid Unicode BCP47 Locale Identifier.
  ///
  /// Some examples of such identifiers: "en", "es-419", "hi-Deva-IN" and
  /// "zh-Hans-CN". See http://www.unicode.org/reports/tr35/ for technical
  /// details.
  String toLanguageTag() => _rawToString('-');

  String _rawToString(String separator) {
    final StringBuffer out = StringBuffer(languageCode);
    if (scriptCode != null && scriptCode!.isNotEmpty)
      out.write('$separator$scriptCode');
    if (_countryCode != null && _countryCode!.isNotEmpty)
      out.write('$separator$countryCode');
    return out.toString();
  }
}

/// A view into which a Flutter [Scene] is drawn.
///
/// Each [FlutterView] has its own layer tree that is rendered into an area
/// inside of the [FlutterWindow] whenever [render] is called with a [Scene].
///
/// New views can be created by the [PlatformDispatcher], using
/// [PlatformDispatcher.createView].
///
/// ## Insets and Padding
///
/// {@animation 300 300 https://flutter.github.io/assets-for-api-docs/assets/widgets/window_padding.mp4}
///
/// In this illustration, the black areas represent system UI that the app
/// cannot draw over. The red area represents view padding that the view may not
/// be able to detect gestures in and may not want to draw in. The grey area
/// represents the system keyboard, which can cover over the bottom view padding
/// when visible.
///
/// The [FlutterWindow.viewInsets] are the physical pixels which the operating
/// system reserves for system UI, such as the keyboard, which would fully
/// obscure any content drawn in that area.
///
/// The [FlutterWindow.viewPadding] are the physical pixels on each side of the
/// display that may be partially obscured by system UI or by physical
/// intrusions into the display, such as an overscan region on a television or a
/// "notch" on a phone. Unlike the insets, these areas may have portions that
/// show the user view-painted pixels without being obscured, such as a
/// notch at the top of a phone that covers only a subset of the area. Insets,
/// on the other hand, either partially or fully obscure the window, such as an
/// opaque keyboard or a partially translucent status bar, which cover an area
/// without gaps.
///
/// The [FlutterWindow.padding] property is computed from both
/// [FlutterWindow.viewInsets] and [FlutterWindow.viewPadding]. It will allow a
/// view inset to consume view padding where appropriate, such as when a phone's
/// keyboard is covering the bottom view padding and so "absorbs" it.
///
/// Clients that want to position elements relative to the view padding
/// regardless of the view insets should use the [FlutterWindow.viewPadding]
/// property, e.g. if you wish to draw a widget at the center of the screen with
/// respect to the iPhone "safe area" regardless of whether the keyboard is
/// showing.
///
/// [FlutterWindow.padding] is useful for clients that want to know how much
/// padding should be accounted for without concern for the current inset(s)
/// state, e.g. determining whether a gesture should be considered for scrolling
/// purposes. This value varies based on the current state of the insets. For
/// example, a visible keyboard will consume all gestures in the bottom part of
/// the [FlutterWindow.viewPadding] anyway, so there is no need to account for
/// that in the [FlutterWindow.padding], which is always safe to use for such
/// calculations.
///
/// See also:
///
///  * [FlutterWindow], a special case of a [FlutterView] that is represented on
///    the platform as a separate window which can host other [FlutterView]s.
abstract class FlutterView {
  /// The platform dispatcher that this view is registered with, and gets its
  /// information from.
  PlatformDispatcher get platformDispatcher;

  /// The configuration of this view.
  ViewConfiguration get viewConfiguration;

  /// The number of device pixels for each logical pixel for the screen this
  /// view is displayed on.
  ///
  /// This number might not be a power of two. Indeed, it might not even be an
  /// integer. For example, the Nexus 6 has a device pixel ratio of 3.5.
  ///
  /// Device pixels are also referred to as physical pixels. Logical pixels are
  /// also referred to as device-independent or resolution-independent pixels.
  ///
  /// By definition, there are roughly 38 logical pixels per centimeter, or
  /// about 96 logical pixels per inch, of the physical display. The value
  /// returned by [devicePixelRatio] is ultimately obtained either from the
  /// hardware itself, the device drivers, or a hard-coded value stored in the
  /// operating system or firmware, and may be inaccurate, sometimes by a
  /// significant margin.
  ///
  /// The Flutter framework operates in logical pixels, so it is rarely
  /// necessary to directly deal with this property.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  double get devicePixelRatio => viewConfiguration.screen.configuration.devicePixelRatio;

  /// The dimensions and location of the rectangle into which the scene rendered
  /// in this view will be drawn on the screen, in physical pixels.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// At startup, the size and location of the view may not be known before Dart
  /// code runs. If this value is observed early in the application lifecycle,
  /// it may report [Rect.zero].
  ///
  /// This value does not take into account any on-screen keyboards or other
  /// system UI. The [padding] and [viewInsets] properties provide a view into
  /// how much of each side of the view may be obscured by system UI.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  Rect get physicalGeometry => viewConfiguration.geometry;

  /// The dimensions of the rectangle into which the scene rendered in this view
  /// will be drawn on the screen, in physical pixels.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// At startup, the size of the view may not be known before Dart code runs.
  /// If this value is observed early in the application lifecycle, it may
  /// report [Size.zero].
  ///
  /// This value does not take into account any on-screen keyboards or other
  /// system UI. The [padding] and [viewInsets] properties provide information
  /// about how much of each side of the view may be obscured by system UI.
  ///
  /// This value is the same as [physicalGeometry.size].
  ///
  /// See also:
  ///
  ///  * [physicalGeometry], which reports the location of the view as well as
  ///    its size.
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  Size get physicalSize => viewConfiguration.geometry.size;

  /// The physical depth is the maximum elevation that the `FlutterView` allows.
  ///
  /// Physical layers drawn at or above this elevation will have their elevation
  /// clamped to this value. This can happen if the physical layer itself has
  /// an elevation larger than available depth, or if some ancestor of the layer
  /// causes it to have a cumulative elevation that is larger than the available
  /// depth.
  ///
  /// The default value is [double.maxFinite], which is used for platforms that
  /// do not specify a maximum elevation. This property is currently only
  /// expected to be set to a non-default value on the Fuchsia platform.
  double get physicalDepth => viewConfiguration.depth;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the view can render, but over which the operating system will likely
  /// place system UI, such as the keyboard, that fully obscures any content.
  ///
  /// When this property changes, [onMetricsChanged] is called.
  ///
  /// The relationship between this [FlutterWindow.viewInsets],
  /// [FlutterWindow.viewPadding], and [FlutterWindow.padding] are described in
  /// more detail in the documentation for [FlutterWindow].
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  ///  * [Scaffold], which automatically applies the view insets in material
  ///    design applications.
  WindowPadding get viewInsets => viewConfiguration.viewInsets;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the view can render, but which may be partially obscured by system
  /// UI (such as the system notification area), or or physical intrusions in
  /// the display (e.g. overscan regions on television screens or phone sensor
  /// housings).
  ///
  /// Unlike [FlutterWindow.padding], this value does not change relative to
  /// [FlutterWindow.viewInsets]. For example, on an iPhone X, it will not
  /// change in response to the soft keyboard being visible or hidden, whereas
  /// [FlutterWindow.padding] will.
  ///
  /// When this property changes, [onMetricsChanged] is called.
  ///
  /// The relationship between this [FlutterWindow.viewInsets],
  /// [FlutterWindow.viewPadding], and [FlutterWindow.padding] are described in
  /// more detail in the documentation for [FlutterWindow].
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  ///  * [Scaffold], which automatically applies the padding in material design
  ///    applications.
  WindowPadding get viewPadding => viewConfiguration.viewPadding;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the view can render, but where the operating system will consume
  /// input gestures for the sake of system navigation.
  ///
  /// For example, an operating system might use the vertical edges of the
  /// screen, where swiping inwards from the edges takes users backward
  /// through the history of screens they previously visited.
  ///
  /// When this property changes, [onMetricsChanged] is called.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  WindowPadding get systemGestureInsets => viewConfiguration.systemGestureInsets;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the view can render, but which may be partially obscured by system
  /// UI (such as the system notification area), or or physical intrusions in
  /// the display (e.g. overscan regions on television screens or phone sensor
  /// housings).
  ///
  /// This value is calculated by taking `max(0.0, FlutterView.viewPadding -
  /// FlutterView.viewInsets)`. This will treat a system IME that increases the
  /// bottom inset as consuming that much of the bottom padding. For example, on
  /// an iPhone X, [FlutterView.padding.bottom] is the same as
  /// [FlutterView.viewPadding.bottom] when the soft keyboard is not drawn (to
  /// account for the bottom soft button area), but will be `0.0` when the soft
  /// keyboard is visible.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// The relationship between this [FlutterWindow.viewInsets],
  /// [FlutterWindow.viewPadding], and [FlutterWindow.padding] are described in
  /// more detail in the documentation for [FlutterWindow].
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  ///  * [Scaffold], which automatically applies the padding in material design
  ///    applications.
  WindowPadding get padding => viewConfiguration.padding;

  /// Updates the view's rendering on the GPU with the newly provided
  /// [Scene].
  ///
  /// This function must be called within the scope of the
  /// [PlatformDispatcher.onBeginFrame] or [PlatformDispatcher.onDrawFrame]
  /// callbacks being invoked. If this function is called a second time during a
  /// single [PlatformDispatcher.onBeginFrame]/[PlatformDispatcher.onDrawFrame]
  /// callback sequence or called outside the scope of those callbacks, the call
  /// will be ignored.
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
  void render(Scene scene) => platformDispatcher.render(scene, this);

  /// Dispose of this view, closing it permanently.
  ///
  /// This function should be called in response to a call to
  /// [PlatformDispatcher.onViewDisposed], or to permanently dispose of a view
  /// that the application would like to close.
  void dispose() => platformDispatcher.disposeView(this);
}

/// A type of [FlutterView] that can be hosted inside of a [FlutterWindow], into
/// which a Flutter [Scene] is drawn.
///
/// A [FlutterWindowView] has its own layer tree that is rendered into an area
/// inside of a [FlutterWindow] when [render] is called with a [Scene].
///
/// A [FlutterWindow] is a subclass of a [FlutterView] that is a separate window
/// which can host other [FlutterView]s.
///
/// A `FlutterWindowView` can only be created by [PlatformDispatcher.createView]
/// where the requested configuration includes a parent [FlutterWindow] set as
/// the [ViewConfiguration.window].
class FlutterWindowView extends FlutterView {
  FlutterWindowView._({Object viewId, this.platformDispatcher})
      : _viewId = viewId;

  /// The opaque ID for this view.
  final Object _viewId;

  @override
  final PlatformDispatcher platformDispatcher;

  @override
  ViewConfiguration get viewConfiguration {
    assert(platformDispatcher._viewConfigurations.containsKey(_viewId));
    return platformDispatcher._viewConfigurations[_viewId];
  }
}

/// A top-level platform window displaying a Flutter layer tree drawn from a
/// [Scene].
///
/// The current list of all Flutter views for the application is available from
/// `WidgetsBinding.instance.platformDispatcher.views`. Only views that are of
/// type [FlutterWindow] are top level platform windows.
///
/// There is also a [PlatformDispatcher.instance] singleton object in `dart:ui`
/// if `WidgetsBinding` is unavailable, but we strongly advise avoiding a static
/// reference to it. See the documentation for [PlatformDispatcher.instance] for
/// more details about why it should be avoided.
///
/// See also:
///
///  * [PlatformDispatcher], which manages the current list of [FlutterView]
///    (and thus [FlutterWindow]) instances.
class FlutterWindow extends FlutterView {
  FlutterWindow._({Object windowId, this.platformDispatcher})
      : _windowId = windowId;

  /// The opaque ID for this view.
  final Object _windowId;

  @override
  final PlatformDispatcher platformDispatcher;

  /// The configuration of this view.
  @override
  ViewConfiguration get viewConfiguration {
    assert(platformDispatcher._viewConfigurations.containsKey(_windowId));
    return platformDispatcher._viewConfigurations[_windowId];
  }
}

/// A [FlutterWindow] that includes access to setting callbacks and retrieving
/// properties that reside on the [PlatformDispatcher].
///
/// It is the type of the legacy global [window] singleton, and the
/// `WidgetsBinding.instance.window` singleton.
///
/// This class provides backward compatibility with code that was written before
/// Flutter supported multiple top level windows. New code should refer to the
/// [WidgetsBinding.instance.platformDispatcher] or [FlutterWindow] class
/// directly.
///
/// There is also a [PlatformDispatcher.instance] singleton object in `dart:ui`
/// if `WidgetsBinding` is unavailable. But we strongly advise avoiding a static
/// reference to it. See the documentation for [PlatformDispatcher.instance] for
/// more details about why it should be avoided.
class SingletonFlutterWindow extends FlutterWindow {
  SingletonFlutterWindow._({Object windowId, PlatformDispatcher platformDispatcher})
      : super._(windowId: windowId, platformDispatcher: platformDispatcher);

  /// A callback that is invoked whenever the [devicePixelRatio],
  /// [physicalSize], [padding], [viewInsets], or [systemGestureInsets]
  /// values change.
  ///
  /// {@template flutter.lib.ui.window.forwardWarning}
  ///
  /// See [PlatformDispatcher.onMetricsChanged] for more information.
  VoidCallback? get onMetricsChanged => platformDispatcher.onMetricsChanged;
  set onMetricsChanged(VoidCallback? callback) {
    platformDispatcher.onMetricsChanged = callback;
  }

  /// The system-reported default locale of the device.
  ///
  /// {@template flutter.lib.ui.window.accessorForwardWarning}
  /// Accessing this value returns the value contained in the
  /// [PlatformDispatcher] singleton, so instead of getting it from here, you
  /// should consider getting it from
  /// `WidgetsBinding.instance.platformDispatcher` instead (or, as a last resort
  /// when `WidgetsBinding` isn't available, from
  /// [PlatformDispatcher.instance]). The reason this value forwards to the
  /// [PlatformDispatcher] is to avoid breaking code that was written before
  /// Flutter supported multiple windows.
  /// {@endtemplate}
  ///
  /// This establishes the language and formatting conventions that window
  /// should, if possible, use to render their user interface.
  ///
  /// This is the first locale selected by the user and is the user's
  /// primary locale (the locale the device UI is displayed in)
  ///
  /// This is equivalent to `locales.first` and will provide an empty non-null
  /// locale if the [locales] list has not been set or is empty.
  Locale? get locale => platformDispatcher.locale;

  /// The full system-reported supported locales of the device.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
  ///
  /// This establishes the language and formatting conventions that window
  /// should, if possible, use to render their user interface.
  ///
  /// The list is ordered in order of priority, with lower-indexed locales being
  /// preferred over higher-indexed ones. The first element is the primary [locale].
  ///
  /// The [onLocaleChanged] callback is called whenever this value changes.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  List<Locale>? get locales => platformDispatcher.locales;

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
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onLocaleChanged => platformDispatcher.onLocaleChanged;
  set onLocaleChanged(VoidCallback? callback) {
    platformDispatcher.onLocaleChanged = callback;
  }

  /// The lifecycle state immediately after dart isolate initialization.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
  ///
  /// This property will not be updated as the lifecycle changes.
  ///
  /// It is used to initialize [SchedulerBinding.lifecycleState] at startup
  /// with any buffered lifecycle state events.
  String get initialLifecycleState => platformDispatcher.initialLifecycleState;

  /// The system-reported text scale.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
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
  double get textScaleFactor => platformDispatcher.textScaleFactor;

  /// The setting indicating whether time should always be shown in the 24-hour
  /// format.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
  ///
  /// This option is used by [showTimePicker].
  bool get alwaysUse24HourFormat => platformDispatcher.alwaysUse24HourFormat;

  /// A callback that is invoked whenever [textScaleFactor] changes value.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onTextScaleFactorChanged => platformDispatcher.onTextScaleFactorChanged;
  set onTextScaleFactorChanged(VoidCallback? callback) {
    platformDispatcher.onTextScaleFactorChanged = callback;
  }

  /// The setting indicating the current brightness mode of the host platform.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
  ///
  /// If the platform has no preference, [platformBrightness] defaults to
  /// [Brightness.light].
  Brightness get platformBrightness => platformDispatcher.platformBrightness;

  /// A callback that is invoked whenever [platformBrightness] changes value.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback? get onPlatformBrightnessChanged => platformDispatcher.onPlatformBrightnessChanged;
  set onPlatformBrightnessChanged(VoidCallback? callback) {
    platformDispatcher.onPlatformBrightnessChanged = callback;
  }

  /// A callback that is invoked to notify the window that it is an appropriate
  /// time to provide a scene using the [SceneBuilder] API and the [render]
  /// method.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// When possible, this is driven by the hardware VSync signal. This is only
  /// called if [scheduleFrame] has been called since the last time this
  /// callback was invoked.
  ///
  /// The [onDrawFrame] callback is invoked immediately after [onBeginFrame],
  /// after draining any microtasks (e.g. completions of any [Future]s) queued
  /// by the [onBeginFrame] handler.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  ///  * [RendererBinding], the Flutter framework class which manages layout and
  ///    painting.
  FrameCallback? get onBeginFrame => platformDispatcher.onBeginFrame;
  set onBeginFrame(FrameCallback? callback) {
    platformDispatcher.onBeginFrame = callback;
  }

  /// A callback that is invoked for each frame after [onBeginFrame] has
  /// completed and after the microtask queue has been drained.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// This can be used to implement a second phase of frame rendering that
  /// happens after any deferred work queued by the [onBeginFrame] phase.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  ///  * [RendererBinding], the Flutter framework class which manages layout and
  ///    painting.
  VoidCallback? get onDrawFrame => platformDispatcher.onDrawFrame;
  set onDrawFrame(VoidCallback? callback) {
    platformDispatcher.onDrawFrame = callback;
  }

  /// A callback that is invoked to report the [FrameTiming] of recently
  /// rasterized frames.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// It's prefered to use [SchedulerBinding.addTimingsCallback] than to use
  /// [FlutterWindow.onReportTimings] directly because
  /// [SchedulerBinding.addTimingsCallback] allows multiple callbacks.
  ///
  /// This can be used to see if the window has missed frames (through
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
  TimingsCallback? get onReportTimings => platformDispatcher.onReportTimings;
  set onReportTimings(TimingsCallback? callback) {
    platformDispatcher.onReportTimings = callback;
  }

  /// A callback that is invoked when pointer data is available.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [GestureBinding], the Flutter framework class which manages pointer
  ///    events.
  PointerDataPacketCallback? get onPointerDataPacket => platformDispatcher.onPointerDataPacket;
  set onPointerDataPacket(PointerDataPacketCallback? callback) {
    platformDispatcher.onPointerDataPacket = callback;
  }

  /// The route or path that the embedder requested when the application was
  /// launched.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
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
  String get initialRouteName => platformDispatcher.initialRouteName;

  /// Requests that, at the next appropriate opportunity, the [onBeginFrame]
  /// and [onDrawFrame] callbacks be invoked.
  ///
  /// {@template flutter.lib.ui.window.functionForwardWarning}
  /// Calling this function calls the same function on the [PlatformDispatcher]
  /// singleton, so instead of calling it here, you should consider calling it
  /// on `WidgetsBinding.instance.platformDispatcher` instead (or, as a last
  /// resort when `WidgetsBinding` isn't available, on
  /// [PlatformDispatcher.instance]). The reason this function forwards to the
  /// [PlatformDispatcher] is to avoid breaking code that was written before
  /// Flutter supported multiple windows.
  /// {@endtemplate}
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  void scheduleFrame() => platformDispatcher.scheduleFrame();

  /// Whether the user has requested that [updateSemantics] be called when
  /// the semantic contents of window changes.
  ///
  /// {@macro flutter.lib.ui.window.accessorForwardWarning}
  ///
  /// The [onSemanticsEnabledChanged] callback is called whenever this value
  /// changes.
  bool get semanticsEnabled => platformDispatcher.semanticsEnabled;

  /// A callback that is invoked when the value of [semanticsEnabled] changes.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  VoidCallback? get onSemanticsEnabledChanged => platformDispatcher.onSemanticsEnabledChanged;
  set onSemanticsEnabledChanged(VoidCallback? callback) {
    platformDispatcher.onSemanticsEnabledChanged = callback;
  }

  /// A callback that is invoked whenever the user requests an action to be
  /// performed.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// This callback is used when the user expresses the action they wish to
  /// perform based on the semantics supplied by [updateSemantics].
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  SemanticsActionCallback? get onSemanticsAction => platformDispatcher.onSemanticsAction;
  set onSemanticsAction(SemanticsActionCallback? callback) {
    platformDispatcher.onSemanticsAction = callback;
  }

  /// Additional accessibility features that may be enabled by the platform.
  AccessibilityFeatures get accessibilityFeatures => platformDispatcher.accessibilityFeatures;

  /// A callback that is invoked when the value of [accessibilityFeatures] changes.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  VoidCallback? get onAccessibilityFeaturesChanged => platformDispatcher.onAccessibilityFeaturesChanged;
  set onAccessibilityFeaturesChanged(VoidCallback? callback) {
    platformDispatcher.onAccessibilityFeaturesChanged = callback;
  }

  /// Change the retained semantics data about this window.
  ///
  /// {@macro flutter.lib.ui.window.functionForwardWarning}
  ///
  /// If [semanticsEnabled] is true, the user has requested that this function
  /// be called whenever the semantic content of this window changes.
  ///
  /// In either case, this function disposes the given update, which means the
  /// semantics update cannot be used further.
  void updateSemantics(SemanticsUpdate update) => platformDispatcher.updateSemantics(update);

  /// Sends a message to a platform-specific plugin.
  ///
  /// {@macro flutter.lib.ui.window.functionForwardWarning}
  ///
  /// The `name` parameter determines which plugin receives the message. The
  /// `data` parameter contains the message payload and is typically UTF-8
  /// encoded JSON but can be arbitrary data. If the plugin replies to the
  /// message, `callback` will be called with the response.
  ///
  /// The framework invokes [callback] in the same zone in which this method
  /// was called.
  void sendPlatformMessage(String name,
      ByteData? data,
      PlatformMessageResponseCallback? callback) {
    platformDispatcher.sendPlatformMessage(name, data, callback);
  }

  /// Called whenever this window receives a message from a platform-specific
  /// plugin.
  ///
  /// {@macro flutter.lib.ui.window.forwardWarning}
  ///
  /// The `name` parameter determines which plugin sent the message. The `data`
  /// parameter is the payload and is typically UTF-8 encoded JSON but can be
  /// arbitrary data.
  ///
  /// Message handlers must call the function given in the `callback` parameter.
  /// If the handler does not need to respond, the handler should pass null to
  /// the callback.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  PlatformMessageCallback? get onPlatformMessage => platformDispatcher.onPlatformMessage;
  set onPlatformMessage(PlatformMessageCallback? callback) {
    platformDispatcher.onPlatformMessage = callback;
  }
}

/// Additional accessibility features that may be enabled by the platform.
///
/// It is not possible to enable these settings from Flutter, instead they are
/// used by the platform to indicate that additional accessibility features are
/// enabled.
//
// When changes are made to this class, the equivalent APIs in each of the
// embedders *must* be updated.
class AccessibilityFeatures {
  const AccessibilityFeatures._(this._index);

  static const int _kAccessibleNavigation = 1 << 0;
  static const int _kInvertColorsIndex = 1 << 1;
  static const int _kDisableAnimationsIndex = 1 << 2;
  static const int _kBoldTextIndex = 1 << 3;
  static const int _kReduceMotionIndex = 1 << 4;
  static const int _kHighContrastIndex = 1 << 5;

  // A bitfield which represents each enabled feature.
  final int _index;

  /// Whether there is a running accessibility service which is changing the
  /// interaction model of the device.
  ///
  /// For example, TalkBack on Android and VoiceOver on iOS enable this flag.
  bool get accessibleNavigation => _kAccessibleNavigation & _index != 0;

  /// The platform is inverting the colors of the application.
  bool get invertColors => _kInvertColorsIndex & _index != 0;

  /// The platform is requesting that animations be disabled or simplified.
  bool get disableAnimations => _kDisableAnimationsIndex & _index != 0;

  /// The platform is requesting that text be rendered at a bold font weight.
  ///
  /// Only supported on iOS.
  bool get boldText => _kBoldTextIndex & _index != 0;

  /// The platform is requesting that certain animations be simplified and
  /// parallax effects removed.
  ///
  /// Only supported on iOS.
  bool get reduceMotion => _kReduceMotionIndex & _index != 0;

  /// The platform is requesting that UI be rendered with darker colors.
  ///
  /// Only supported on iOS.
  bool get highContrast => _kHighContrastIndex & _index != 0;

  @override
  String toString() {
    final List<String> features = <String>[];
    if (accessibleNavigation)
      features.add('accessibleNavigation');
    if (invertColors)
      features.add('invertColors');
    if (disableAnimations)
      features.add('disableAnimations');
    if (boldText)
      features.add('boldText');
    if (reduceMotion)
      features.add('reduceMotion');
    if (highContrast)
      features.add('highContrast');
    return 'AccessibilityFeatures$features';
  }

  @override
  bool operator ==(dynamic other) {
    if (other.runtimeType != runtimeType)
      return false;
    return other is AccessibilityFeatures
        && other._index == _index;
  }

  @override
  int get hashCode => _index.hashCode;
}

/// Describes the contrast of a theme or color palette.
enum Brightness {
  /// The color is dark and will require a light text color to achieve readable
  /// contrast.
  ///
  /// For example, the color might be dark grey, requiring white text.
  dark,

  /// The color is light and will require a dark text color to achieve readable
  /// contrast.
  ///
  /// For example, the color might be bright white, requiring black text.
  light,
}

/// The [SingletonFlutterWindow] representing the "only" window on platforms
/// where there is only one window, such as single-display mobile devices.
///
/// This is a legacy object for code that was written before Flutter supported
/// multiple windows. New code should use the API on
/// `WidgetsBinding.instance.platformDispatcher`, or, where that is not
/// available, on [PlatformDispatcher.instance].
///
/// On platforms with more than one window, this window typically represents the
/// first window created, but it may not be visible.
///
/// Please try to avoid statically referencing this and instead use a
/// binding for dependency resolution such as `WidgetsBinding.instance.window`.
///
/// Static access of this "window" object means that Flutter has few, if any
/// options to fake or mock the given object in tests. Even in cases where Dart
/// offers special language constructs to forcefully shadow such properties,
/// those mechanisms would only be reasonable for tests and they would not be
/// reasonable for a future of Flutter where we legitimately want to select an
/// appropriate implementation at runtime.
///
/// The only place that using `WidgetsBinding.instance.platformDispatcher` is
/// inappropriate is if access to these APIs is required before invoking
/// `runApp()`. In that case, it is acceptable (though unfortunate) to use the
/// [PlatformDispatcher.instance] object statically.
///
/// See also:
///
///  * [PlatformDispatcher.views], contains the current list of Flutter windows
///    belonging to the application.
final SingletonFlutterWindow window = SingletonFlutterWindow._(windowId: 0, platformDispatcher: PlatformDispatcher.instance);
