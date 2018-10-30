// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

/// Signature of callbacks that have no arguments and return no data.
typedef VoidCallback = void Function();

/// Signature for [Window.onBeginFrame].
typedef FrameCallback = void Function(Duration duration);

/// Signature for [Window.onPointerDataPacket].
typedef PointerDataPacketCallback = void Function(PointerDataPacket packet);

/// Signature for [Window.onSemanticsAction].
typedef SemanticsActionCallback = void Function(int id, SemanticsAction action, ByteData args);

/// Signature for responses to platform messages.
///
/// Used as a parameter to [Window.sendPlatformMessage] and
/// [Window.onPlatformMessage].
typedef PlatformMessageResponseCallback = void Function(ByteData data);

/// Signature for [Window.onPlatformMessage].
typedef PlatformMessageCallback = void Function(String name, ByteData data, PlatformMessageResponseCallback callback);

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
  /// [Window.onBeginFrame] and [Window.onDrawFrame] callbacks.
  ///
  /// Android apps in this state should assume that they may enter the
  /// [suspending] state at any time.
  paused,

  /// The application will be suspended momentarily.
  ///
  /// When the application is in this state, the engine will not call the
  /// [Window.onBeginFrame] and [Window.onDrawFrame] callbacks.
  ///
  /// On iOS, this state is currently unused.
  suspending,
}

/// A representation of distances for each of the four edges of a rectangle,
/// used to encode the view insets and padding that applications should place
/// around their user interface, as exposed by [Window.viewInsets] and
/// [Window.padding]. View insets and padding are preferably read via
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
  const WindowPadding._({ this.left, this.top, this.right, this.bottom });

  /// The distance from the left edge to the first unpadded pixel, in physical pixels.
  final double left;

  /// The distance from the top edge to the first unpadded pixel, in physical pixels.
  final double top;

  /// The distance from the right edge to the first unpadded pixel, in physical pixels.
  final double right;

  /// The distance from the bottom edge to the first unpadded pixel, in physical pixels.
  final double bottom;

  /// A window padding that has zeros for each edge.
  static const WindowPadding zero = const WindowPadding._(left: 0.0, top: 0.0, right: 0.0, bottom: 0.0);

  @override
  String toString() {
    return '$runtimeType(left: $left, top: $top, right: $right, bottom: $bottom)';
  }
}

/// An identifier used to select a user's language and formatting preferences.
///
/// This represents a [Unicode Locale
/// Identifier](https://www.unicode.org/reports/tr35/#Unicode_locale_identifier).
///
/// Locales are canonicalized according to the "preferred value" entries in the
/// [IANA Language Subtag
/// Registry](https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry).
/// For example, `const Locale('he')` and `const Locale('iw')` are equal and
/// both have the [languageCode] `he`, because `iw` is a deprecated language
/// subtag that was replaced by the subtag `he`.
///
/// When constructed correctly, instances of this Locale class will produce
/// normalized syntactically correct output, although not necessarily valid
/// (because tags are not validated). See constructor and factory method
/// documentation for details.
///
/// See also:
///
///  * [Window.locale], which specifies the system's currently selected
///    [Locale].
class Locale {
  /// Creates a new Locale object. The first argument is the
  /// primary language subtag, the second is the region subtag.
  ///
  /// For example:
  ///
  /// ```dart
  /// const Locale swissFrench = const Locale('fr', 'CH');
  /// const Locale canadianFrench = const Locale('fr', 'CA');
  /// ```
  ///
  /// The primary language subtag must not be null. The region subtag is
  /// optional.
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
  /// This method only produces standards-compliant instances if valid language
  /// and country codes are provided. Deprecated subtags will be replaced, but
  /// incorrectly cased strings are not corrected.
  ///
  /// Validity is not checked by default, but some methods may throw away
  /// invalid data.
  ///
  /// See also:
  ///
  ///  * [new Locale.fromSubtags], which also allows a [scriptCode] to be
  ///    specified.
  const Locale(
    this._languageCode, [
    this._countryCode,
  ]) : assert(_languageCode != null),
       scriptCode = null,
       _variants = null,
       _extensions = null;

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
  /// Validity is not checked by default, but some methods may throw away
  /// invalid data.
  const Locale.fromSubtags({
    String languageCode = 'und',
    this.scriptCode,
    String countryCode,
  }) : assert(languageCode != null),
       assert(languageCode != ''),
       _languageCode = languageCode,
       assert(scriptCode != ''),
       assert(countryCode != ''),
       _countryCode = countryCode,
       _variants = null,
       _extensions = null;

  // Creates a new Locale object with the specified parts.
  //
  // This is for internal use only. All fields must already be normalized, must
  // already be canonicalized. This method does not modify parameters in any
  // way or do any syntax checking.
  //
  // * language, script and region must be in normalized form.
  // * variants need not be sorted, this constructor performs sorting. Each
  //   variant subtag should already be in normalized form though.
  // * The extensions map must contain only valid key/value pairs. "u" and "t"
  //   keys must be present, with an empty string as value, if there are any
  //   subtags for those singletons.
  Locale._internal(
    String languageCode, {
    this.scriptCode,
    String countryCode,
    List<String> variants,
    collection.LinkedHashMap<String, String> extensions,
  }) : assert(languageCode != null),
       assert(languageCode.length >= 2 && languageCode.length <= 8),
       assert(languageCode.length != 4),
       assert(scriptCode == null || scriptCode.length == 4),
       assert(countryCode == null || (countryCode.length >= 2 && countryCode.length <= 3)),
       _languageCode = languageCode,
       _countryCode = countryCode,
       _variants = (variants != null && variants.isNotEmpty) ? List<String>.from(variants) : null,
       _extensions = (extensions != null && extensions.isNotEmpty) ? <String, String>{} : null
  {
    _variants?.sort();
    if (extensions != null) {
      // Insert extensions in sorted order.
      final List<String> keys = extensions.keys.toList()..sort();
      for (String key in keys) {
        _extensions[key] = extensions[key];
      }
    }
  }

  static final RegExp _reSep = RegExp(r'[-_]');

  /// Parses [Unicode CLDR Locale
  /// Identifiers](https://www.unicode.org/reports/tr35/#Identifiers).
  ///
  /// This method does not parse all BCP 47 tags. See [BCP 47
  /// Conformance](https://www.unicode.org/reports/tr35/#BCP_47_Conformance) for
  /// details.
  ///
  /// TODO: support parsing all BCP 47 tags.
  static Locale parse(String localeId) {
    assert(localeId != null);
    localeId = localeId.toLowerCase();
    if (localeId == 'root')
      return Locale._internal('und');

    final List<String> localeSubtags = localeId.split(_reSep);
    String language, script, region;
    final List<String> variants = <String>[];
    final Map<String, String> extensions = <String, String>{};

    final List<String> problems = <String>[];
    if (_isAlphabetic(localeSubtags[0], 2, 8)
        && localeSubtags[0].length != 4) {
      // language subtag: r'^[a-zA-Z]{2,3}$|^[a-zA-Z]{5,8}$'
      language = _replaceDeprecatedLanguageSubtag(localeSubtags.removeAt(0));
    } else if (_isAlphabetic(localeSubtags[0], 4, 4)) {
      // First subtag is already a script subtag.
      //
      // Identifiers without language subtags aren't valid BCP 47 tags and
      // therefore not intended for general interchange, however they do match
      // the LDML spec.
      language = 'und';
    } else {
      problems.add('"${localeSubtags[0]}" is an invalid language subtag');
    }
    if (localeSubtags.isNotEmpty
        && _isAlphabetic(localeSubtags[0], 4, 4)) {
      // script subtag: r'^[a-zA-Z]{4}$'
      final String s = localeSubtags.removeAt(0);
      script = s.substring(0, 1).toUpperCase() + s.substring(1).toLowerCase();
    }
    if (localeSubtags.isNotEmpty
        && (_isAlphabetic(localeSubtags[0], 2, 2)
            || _isNumeric(localeSubtags[0], 3, 3))) {
      // region subtag: r'^[a-zA-Z]{2}$|^[0-9]{3}$';
      region = localeSubtags.removeAt(0).toUpperCase();
    }
    while (localeSubtags.isNotEmpty && _isVariantSubtag(localeSubtags[0])) {
      variants.add(localeSubtags.removeAt(0));
    }

    // Now we should be into extension territory, localeSubtags[0] should be a singleton.
    if (localeSubtags.isNotEmpty && localeSubtags[0].length > 1) {
      final List<String> mismatched = <String>[];
      if (variants.isEmpty) {
        if (region == null) {
          if (script == null) {
            mismatched.add('script');
          }
          mismatched.add('region');
        }
        mismatched.add('variant');
      }
      problems.add('unrecognised subtag "${localeSubtags[0]}": is not a '
          '${mismatched.join(", ")}');
    }
    _parseExtensions(localeSubtags, extensions, problems);

    if (problems.isNotEmpty) {
      throw FormatException('Locale Identifier $localeId is invalid: '
                            '${problems.join("; ")}.');
    }

    return Locale._internal(
        language,
        scriptCode: script,
        countryCode: region,
        variants: variants,
        extensions: extensions,
    );
  }

  // * All subtags in localeSubtags must already be lowercase.
  //
  // * extensions must be a map with sorted iteration order. LinkedHashMap
  //   preserves order for us.
  static void _parseExtensions(List<String> localeSubtags,
                               collection.LinkedHashMap<String, String> extensions,
                               List<String> problems) {
    final Map<String, String> ext = <String, String>{};
    while (localeSubtags.isNotEmpty) {
      final String singleton = localeSubtags.removeAt(0);
      if (singleton == 'u') {
        bool empty = true;
        // unicode_locale_extensions: collect "(sep attribute)+" attributes.
        final List<String> attributes = <String>[];
        while (localeSubtags.isNotEmpty
               && _isAlphaNumeric(localeSubtags[0], 3, 8)) {
          attributes.add(localeSubtags.removeAt(0));
        }
        if (attributes.isNotEmpty) {
          empty = false;
        }
        if (!ext.containsKey(singleton)) {
          ext[singleton] = attributes.join('-');
        } else {
          problems.add('duplicate singleton: "$singleton"');
        }
        // unicode_locale_extensions: collect "(sep keyword)*".
        while (localeSubtags.isNotEmpty
               && _isUExtensionKey(localeSubtags[0])) {
          empty = false;
          final String key = localeSubtags.removeAt(0);
          final List<String> typeParts = <String>[];
          while (localeSubtags.isNotEmpty
                 && _isAlphaNumeric(localeSubtags[0], 3, 8)) {
            typeParts.add(localeSubtags.removeAt(0));
          }
          if (!ext.containsKey(key)) {
            if (typeParts.length == 1 && typeParts[0] == 'true') {
              ext[key] = '';
            } else {
              ext[key] = typeParts.join('-');
            }
          } else {
            problems.add('duplicate key: $key');
          }
        }
        if (empty) {
          problems.add('empty singleton: $singleton');
        }
      } else if (singleton == 't') {
        bool empty = true;
        // transformed_extensions: grab tlang if it exists.
        final List<String> tlang = <String>[];
        if (localeSubtags.isNotEmpty
            && _isAlphabetic(localeSubtags[0], 2, 8)
            && localeSubtags[0].length != 4) {
          // language subtag
          empty = false;
          tlang.add(localeSubtags.removeAt(0));
          if (localeSubtags.isNotEmpty
              && _isAlphabetic(localeSubtags[0], 4, 4))
            // script subtag
            tlang.add(localeSubtags.removeAt(0));
          if (localeSubtags.isNotEmpty
              && (_isAlphabetic(localeSubtags[0], 2, 2)
                  || _isNumeric(localeSubtags[0], 3, 3)))
            // region subtag: r'^[a-zA-Z]{2}$|^[0-9]{3}$';
            tlang.add(localeSubtags.removeAt(0));
          while (localeSubtags.isNotEmpty && _isVariantSubtag(localeSubtags[0])) {
            tlang.add(localeSubtags.removeAt(0));
          }
        }
        if (!ext.containsKey(singleton)) {
          ext[singleton] = tlang.join('-');
        } else {
          problems.add('duplicate singleton: "$singleton"');
        }
        // transformed_extensions: collect "(sep tfield)*".
        while (localeSubtags.isNotEmpty && _isTExtensionKey(localeSubtags[0])) {
          final String tkey = localeSubtags.removeAt(0);
          final List<String> tvalueParts = <String>[];
          while (localeSubtags.isNotEmpty
                 && _isAlphaNumeric(localeSubtags[0], 3, 8)) {
            tvalueParts.add(localeSubtags.removeAt(0));
          }
          if (tvalueParts.isNotEmpty) {
            empty = false;
            if (!ext.containsKey(tkey)) {
                ext[tkey] = tvalueParts.join('-');
            } else {
              problems.add('duplicate key: $tkey');
            }
          }
        }
        if (empty) {
          problems.add('empty singleton: $singleton');
        }
      } else if (singleton == 'x') {
        // pu_extensions
        final List<String> values = <String>[];
        while (localeSubtags.isNotEmpty
               && _isAlphaNumeric(localeSubtags[0], 1, 8)) {
          values.add(localeSubtags.removeAt(0));
        }
        ext[singleton] = values.join('-');
        if (localeSubtags.isNotEmpty) {
            problems.add('invalid part of private use subtags: "${localeSubtags.join('-')}"');
        }
        break;
      } else if (_isAlphabetic(singleton, 1, 1)) {
        // other_extensions
        final List<String> values = <String>[];
        while (localeSubtags.isNotEmpty
               && _isAlphaNumeric(localeSubtags[0], 2, 8)) {
          values.add(localeSubtags.removeAt(0));
        }
        if (!ext.containsKey(singleton)) {
          ext[singleton] = values.join('-');
        } else {
          problems.add('duplicate singleton: "$singleton"');
        }
      } else {
        problems.add('invalid subtag, should be singleton: "$singleton"');
      }
    }
    final List<String> ks = ext.keys.toList()..sort();
    for (String k in ks) {
      extensions[k] = ext[k];
    }
  }

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
  /// New deprecations in the registry are not automatically picked up by this
  /// library, so this class will not make such changes for deprecations that
  /// are too recent.
  ///
  /// See also:
  ///
  ///  * [new Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  String get languageCode => _replaceDeprecatedLanguageSubtag(_languageCode);
  final String _languageCode;

  // Replaces deprecated language subtags.
  //
  // The subtag must already be lowercase.
  static String _replaceDeprecatedLanguageSubtag(String languageCode) {
    // This switch statement is generated by //flutter/tools/gen_locale.dart
    // Mappings generated for language subtag registry as of 2018-08-08.
    switch (languageCode) {
      case 'in': return 'id'; // Indonesian; deprecated 1989-01-01
      case 'iw': return 'he'; // Hebrew; deprecated 1989-01-01
      case 'ji': return 'yi'; // Yiddish; deprecated 1989-01-01
      case 'jw': return 'jv'; // Javanese; deprecated 2001-08-13
      case 'mo': return 'ro'; // Moldavian, Moldovan; deprecated 2008-11-22
      case 'aam': return 'aas'; // Aramanik; deprecated 2015-02-12
      case 'adp': return 'dz'; // Adap; deprecated 2015-02-12
      case 'aue': return 'ktz'; // =/Kx'au//'ein; deprecated 2015-02-12
      case 'ayx': return 'nun'; // Ayi (China); deprecated 2011-08-16
      case 'bgm': return 'bcg'; // Baga Mboteni; deprecated 2016-05-30
      case 'bjd': return 'drl'; // Bandjigali; deprecated 2012-08-12
      case 'ccq': return 'rki'; // Chaungtha; deprecated 2012-08-12
      case 'cjr': return 'mom'; // Chorotega; deprecated 2010-03-11
      case 'cka': return 'cmr'; // Khumi Awa Chin; deprecated 2012-08-12
      case 'cmk': return 'xch'; // Chimakum; deprecated 2010-03-11
      case 'coy': return 'pij'; // Coyaima; deprecated 2016-05-30
      case 'cqu': return 'quh'; // Chilean Quechua; deprecated 2016-05-30
      case 'drh': return 'khk'; // Darkhat; deprecated 2010-03-11
      case 'drw': return 'prs'; // Darwazi; deprecated 2010-03-11
      case 'gav': return 'dev'; // Gabutamon; deprecated 2010-03-11
      case 'gfx': return 'vaj'; // Mangetti Dune !Xung; deprecated 2015-02-12
      case 'ggn': return 'gvr'; // Eastern Gurung; deprecated 2016-05-30
      case 'gti': return 'nyc'; // Gbati-ri; deprecated 2015-02-12
      case 'guv': return 'duz'; // Gey; deprecated 2016-05-30
      case 'hrr': return 'jal'; // Horuru; deprecated 2012-08-12
      case 'ibi': return 'opa'; // Ibilo; deprecated 2012-08-12
      case 'ilw': return 'gal'; // Talur; deprecated 2013-09-10
      case 'jeg': return 'oyb'; // Jeng; deprecated 2017-02-23
      case 'kgc': return 'tdf'; // Kasseng; deprecated 2016-05-30
      case 'kgh': return 'kml'; // Upper Tanudan Kalinga; deprecated 2012-08-12
      case 'koj': return 'kwv'; // Sara Dunjo; deprecated 2015-02-12
      case 'krm': return 'bmf'; // Krim; deprecated 2017-02-23
      case 'ktr': return 'dtp'; // Kota Marudu Tinagas; deprecated 2016-05-30
      case 'kvs': return 'gdj'; // Kunggara; deprecated 2016-05-30
      case 'kwq': return 'yam'; // Kwak; deprecated 2015-02-12
      case 'kxe': return 'tvd'; // Kakihum; deprecated 2015-02-12
      case 'kzj': return 'dtp'; // Coastal Kadazan; deprecated 2016-05-30
      case 'kzt': return 'dtp'; // Tambunan Dusun; deprecated 2016-05-30
      case 'lii': return 'raq'; // Lingkhim; deprecated 2015-02-12
      case 'lmm': return 'rmx'; // Lamam; deprecated 2014-02-28
      case 'meg': return 'cir'; // Mea; deprecated 2013-09-10
      case 'mst': return 'mry'; // Cataelano Mandaya; deprecated 2010-03-11
      case 'mwj': return 'vaj'; // Maligo; deprecated 2015-02-12
      case 'myt': return 'mry'; // Sangab Mandaya; deprecated 2010-03-11
      case 'nad': return 'xny'; // Nijadali; deprecated 2016-05-30
      case 'ncp': return 'kdz'; // Ndaktup; deprecated 2018-03-08
      case 'nnx': return 'ngv'; // Ngong; deprecated 2015-02-12
      case 'nts': return 'pij'; // Natagaimas; deprecated 2016-05-30
      case 'oun': return 'vaj'; // !O!ung; deprecated 2015-02-12
      case 'pcr': return 'adx'; // Panang; deprecated 2013-09-10
      case 'pmc': return 'huw'; // Palumata; deprecated 2016-05-30
      case 'pmu': return 'phr'; // Mirpur Panjabi; deprecated 2015-02-12
      case 'ppa': return 'bfy'; // Pao; deprecated 2016-05-30
      case 'ppr': return 'lcq'; // Piru; deprecated 2013-09-10
      case 'pry': return 'prt'; // Pray 3; deprecated 2016-05-30
      case 'puz': return 'pub'; // Purum Naga; deprecated 2014-02-28
      case 'sca': return 'hle'; // Sansu; deprecated 2012-08-12
      case 'skk': return 'oyb'; // Sok; deprecated 2017-02-23
      case 'tdu': return 'dtp'; // Tempasuk Dusun; deprecated 2016-05-30
      case 'thc': return 'tpo'; // Tai Hang Tong; deprecated 2016-05-30
      case 'thx': return 'oyb'; // The; deprecated 2015-02-12
      case 'tie': return 'ras'; // Tingal; deprecated 2011-08-16
      case 'tkk': return 'twm'; // Takpa; deprecated 2011-08-16
      case 'tlw': return 'weo'; // South Wemale; deprecated 2012-08-12
      case 'tmp': return 'tyj'; // Tai Mène; deprecated 2016-05-30
      case 'tne': return 'kak'; // Tinoc Kallahan; deprecated 2016-05-30
      case 'tnf': return 'prs'; // Tangshewi; deprecated 2010-03-11
      case 'tsf': return 'taj'; // Southwestern Tamang; deprecated 2015-02-12
      case 'uok': return 'ema'; // Uokha; deprecated 2015-02-12
      case 'xba': return 'cax'; // Kamba (Brazil); deprecated 2016-05-30
      case 'xia': return 'acn'; // Xiandao; deprecated 2013-09-10
      case 'xkh': return 'waw'; // Karahawyana; deprecated 2016-05-30
      case 'xsj': return 'suj'; // Subi; deprecated 2015-02-12
      case 'ybd': return 'rki'; // Yangbye; deprecated 2012-08-12
      case 'yma': return 'lrr'; // Yamphe; deprecated 2012-08-12
      case 'ymt': return 'mtm'; // Mator-Taygi-Karagas; deprecated 2015-02-12
      case 'yos': return 'zom'; // Yos; deprecated 2013-09-10
      case 'yuu': return 'yug'; // Yugh; deprecated 2014-02-28
      default: return languageCode;
    }
  }

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
  ///  * [new Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  final String scriptCode;

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
  /// New deprecations in the registry are not automatically picked up by this
  /// library, so this class will not make such changes for deprecations that
  /// are too recent.
  ///
  /// See also:
  ///
  ///  * [new Locale.fromSubtags], which describes the conventions for creating
  ///    [Locale] objects.
  String get countryCode => _replaceDeprecatedRegionSubtag(_countryCode);
  final String _countryCode;

  // Replaces deprecated region subtags.
  //
  // The subtag must already be uppercase.
  static String _replaceDeprecatedRegionSubtag(String regionCode) {
    // This switch statement is generated by //flutter/tools/gen_locale.dart
    // Mappings generated for language subtag registry as of 2018-08-08.
    switch (regionCode) {
      case 'BU': return 'MM'; // Burma; deprecated 1989-12-05
      case 'DD': return 'DE'; // German Democratic Republic; deprecated 1990-10-30
      case 'FX': return 'FR'; // Metropolitan France; deprecated 1997-07-14
      case 'TP': return 'TL'; // East Timor; deprecated 2002-05-20
      case 'YD': return 'YE'; // Democratic Yemen; deprecated 1990-08-14
      case 'ZR': return 'CD'; // Zaire; deprecated 1997-07-14
      default: return regionCode;
    }
  }

  // Unicode language variant codes.
  //
  // This iterable provides variants normalized to lowercase and in sorted
  // order, as per the Unicode LDML specification.
  //
  // FIXME: decide whether this returns a single string or something fancier.
  Iterable<String> get variants => _variants ?? const <String>[];

  // The private _variants field must have variants in lowercase and already
  // sorted: constructors must construct it as such.
  final List<String> _variants;

  // A map representing all Locale Identifier extensions.
  //
  // Keys in this ordered map must be sorted, and both keys and values must all
  // be lowercase: constructors must construct it as such.
  //
  // This map simultaneously represents T-extensions, U-extensions, other
  // extensions and the private use extensions. Implementation detailsf:
  //
  // * The 't' entry represents the optional "tlang" identifier of the T
  //   Extension. If the T Extension is present but has no tlang value, the 't'
  //   map entry's value must be an empty string.
  // * The 'u' entry represents the optional attributes of the U Extension. They
  //   must be sorted in alphabetical order, separated by hyphens, and be
  //   lowercase. If the U Extension is present but has no attributes, the 'u'
  //   map entry's value must be an empty string.
  // * U-Extension keyword keys and T-Extension fields in the map are directly
  //   entered into this map: their syntax are enough to distinguish
  //   them. (_isUExtensionKey and _isTExtensionKey private methods check this
  //   syntax.)
  // * Other singletons are entered directly into the map, with all
  //   values/attributes associated with that singleton as the map entry's
  //   value.
  final collection.LinkedHashMap<String, String> _extensions;

  // TMP FIXME
  collection.LinkedHashMap<String, String> get myexthelper => _extensions;

  // Produces the Unicode BCP47 Locale Identifier for this locale.
  //
  // If the unnamed constructor was used with bad parameters, the result might
  // not be standards-compliant.
  // https://www.unicode.org/reports/tr35/#Unicode_locale_identifier
  //
  // FIXME/TODO: this must not be submitted as a public function until we've
  // made a final decision on toString() behaviour.
  String toLanguageTag() {
    final StringBuffer out = StringBuffer(languageCode);
    if (scriptCode != null) {
      out.write('-$scriptCode');
    }
    if (_countryCode != null && _countryCode != '') {
      out.write('-$countryCode');
    }
    for (String v in variants) {
      out.write('-$v');
    }
    if (_extensions != null && _extensions.isNotEmpty) {
      out.write(_extensionsToString(_extensions));
    }
    return out.toString();
  }

  // Formats the extension map into a partial Unicode Locale Identifier.
  //
  // This covers everything after the unicode_language_id, and returns a string
  // starting with a hyphen. Returns '' if passed null or an empty extensions
  // map.
  static String _extensionsToString(collection.LinkedHashMap<String, String> extensions) {
    if (extensions == null || extensions.isEmpty) {
      return '';
    }
    String uAttributes;
    String tLang;
    final StringBuffer uOut = StringBuffer();
    final StringBuffer tOut = StringBuffer();
    final StringBuffer result = StringBuffer();
    final StringBuffer resultVWYZ = StringBuffer();

    for (MapEntry<String, String> entry in extensions.entries) {
      if (entry.key.length == 1) {
        final int letter = entry.key.codeUnitAt(0) - 0x61;  // Subtracting 'a'
        if (letter < 0 || letter >= 26) {
          throw UnimplementedError('Unexpected key in extensions map: $entry');
        } else if (letter < 19) {  // 'a' to 's' (19th letter)
          result.write('-${entry.key}');
          if (entry.value.isNotEmpty)
            result.write('-${entry.value}');
        } else if (letter == 19) {  // t
          tLang = entry.value;
        } else if (letter == 20) {  // u
          uAttributes = entry.value;
        } else if (letter != 23) {  // not x: vwyz
          resultVWYZ.write('-${entry.key}');
          if (entry.value.isNotEmpty)
            resultVWYZ.write('-${entry.value}');
        }
      } else if (_isUExtensionKey(entry.key)) {
        // unicode_locale_extensions
        if (entry.value == 'true' || entry.value == '') {
          uOut.write('-${entry.key}');
        } else {
          uOut.write('-${entry.key}-${entry.value}');
        }
      } else if (_isTExtensionKey(entry.key)) {
        // transformed_extensions
        tOut.write('-${entry.key}');
        // TODO: this is not standards compliant. What do we want to do with
        // this case? Drop entry.key like we drop empty t and u singletons?
        // Or simply ensure we don't ever create such an instance?
        if (entry.value.isNotEmpty)
          tOut.write('-${entry.value}');
      } else {
        throw UnimplementedError('Unexpected key in extensions map: $entry');
      }
    }
    if (tLang != null || tOut.isNotEmpty) {
      result.write('-t');
      if (tLang != null)
        result.write('-$tLang');
      result.write(tOut.toString());
    }
    if (uAttributes != null || uOut.isNotEmpty) {
      result.write('-u');
      if (uAttributes != null && uAttributes.isNotEmpty)
        result.write('-$uAttributes');
      result.write(uOut.toString());
    }
    if (resultVWYZ.isNotEmpty)
      result.write(resultVWYZ.toString());
    if (extensions.containsKey('x')) {
      result.write('-x');
      if (extensions['x'].isNotEmpty) {
        result.write('-${extensions["x"]}');
      }
    }
    return result.toString();
  }

  // Returns true if s is a string made of lower-case alphabetic characters
  // (a-z) or digits (0-9), with specified min and max lengths.
  //
  // Benchmarks show that doing this with a lookup map is faster. This function
  // chooses to not keep the needed 256 byte string around though.
  static bool _isAlphaNumeric(String s, int minLength, int maxLength) {
    if (s.length < minLength || s.length > maxLength)
      return false;
    for (int i = 0; i < s.length; i++) {
      final int char = s.codeUnitAt(i);
      // 0-9: 0x30-0x39.
      if (char ^ 0x30 <= 9)
        continue;
      // a-z: 0x61-0x7A
      if ((char - 0x61) & 0xFFFF >= 26)
        return false;
    }
    return true;
  }

  // Returns true if s is a purely lower-case alphabetic (a-z) string with
  // specified min and max lengths.
  static bool _isAlphabetic(String s, int minLength, int maxLength) {
    if (s.length < minLength || s.length > maxLength)
      return false;
    for (int i = 0; i < s.length; i++) {
      final int char = s.codeUnitAt(i);
      // a-z: 0x61-0x7A
      if ((char - 0x61) & 0xFFFF >= 26)
        return false;
    }
    return true;
  }

  // Returns true if s is a string consisting of 3 digits (0-9).
  static bool _isNumeric(String s, int minLength, int maxLength) {
    if (s.length < minLength || s.length > maxLength)
      return false;
    for (int i = 0; i < s.length; i++) {
      // codeUnitAt returns a 16-bit number. Dart's default implementation has
      // 64-bit ints, so this will be a positive number.
      final int char = s.codeUnitAt(i);
      // 0-9: 0x30-0x39.
      if (char ^ 0x30 > 9)
        return false;
    }
    return true;
  }

  // Checks that the specified string matches the variant subtag syntax. Does
  // not check the list of valid subtags!
  //
  // r'^[a-zA-Z0-9]{5,8}$|^[0-9][a-zA-Z0-9]{3}$'
  static bool _isVariantSubtag(String s) {
    if (!_isAlphaNumeric(s, 4, 8))
      return false;
    if (s.length == 4 && !_isNumeric(s[0], 1, 1))
      return false;
    return true;
  }

  // Checks that the specified string matches the syntax of U extension
  // keys. Does not check that it is a valid key!
  //
  // r'^[a-zA-Z0-9][a-zA-Z]\$'
  static bool _isUExtensionKey(String s) {
    if (s.length != 2)
      return false;
    if (!_isAlphaNumeric(s[0], 1, 1))
      return false;
    if (!_isAlphabetic(s[1], 1, 1))
      return false;
    return true;
  }

  // Checks that the specified string matches the syntax of T extension
  // keys. Does not check that it is a valid key!
  //
  // r'^[a-zA-Z][0-9]\$'
  static bool _isTExtensionKey(String s) {
    if (s.length != 2)
      return false;
    if (!_isAlphabetic(s[0], 1, 1))
      return false;
    if (!_isNumeric(s[1], 1, 1))
      return false;
    return true;
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other is! Locale)
      return false;
    final Locale typedOther = other;

    // Comparing Lists, Sets and Maps requires reimplementing functionality in
    // package:collection. Variants and extensions are rare, so we convert them
    // to canonical/normalized strings for comparison.
    return languageCode == typedOther.languageCode
        && scriptCode == typedOther.scriptCode
        && countryCode == typedOther.countryCode
        && _listEquals<String>(_variants, typedOther._variants)
        && _mapEquals<String, String>(_extensions, typedOther._extensions);
  }

  bool _listEquals<T>(List<T> a, List<T> b) {
    if (a == null)
      return b == null;
    if (b == null || a.length != b.length)
      return false;
    for (int index = 0; index < a.length; index += 1) {
      if (a[index] != b[index])
        return false;
    }
    return true;
  }

  bool _mapEquals<T1, T2>(Map<T1, T2> a, Map<T1, T2> b) {
    if (a == null)
      return b == null;
    if (b == null || a.length != b.length)
      return false;
    for (T1 k in a.keys) {
      if (a[k] != b[k])
        return false;
    }
    return true;
  }

  @override
  int get hashCode => hashValues(languageCode, scriptCode, countryCode, hashList(_variants), hashMap(_extensions));

  /// Produces a non-BCP47 Unicode Locale Identifier for this locale.
  ///
  /// This Locale Identifier uses underscores as separator for historical
  /// reasons. Use toLanguageTag() instead, it produces a Unicode BCP47 Locale
  /// Identifier as recommended for general interchange.
  @override
  String toString() {
    String identifier = toLanguageTag().replaceAll('-', '_');
    if (_countryCode == '' && identifier == languageCode) {
      // Not standards-compliant, but kept for legacy reasons. Only the const
      // unnamed constructor should be able to create instances like these.
      identifier = '${languageCode}_';
    }
    return '$identifier';
  }
}

/// The most basic interface to the host operating system's user interface.
///
/// There is a single Window instance in the system, which you can
/// obtain from the [window] property.
class Window {
  Window._();

  /// The number of device pixels for each logical pixel. This number might not
  /// be a power of two. Indeed, it might not even be an integer. For example,
  /// the Nexus 6 has a device pixel ratio of 3.5.
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
  double get devicePixelRatio => _devicePixelRatio;
  double _devicePixelRatio = 1.0;

  /// The dimensions of the rectangle into which the application will be drawn,
  /// in physical pixels.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// At startup, the size of the application window may not be known before Dart
  /// code runs. If this value is observed early in the application lifecycle,
  /// it may report [Size.zero].
  ///
  /// This value does not take into account any on-screen keyboards or other
  /// system UI. The [padding] and [viewInsets] properties provide a view into
  /// how much of each side of the application may be obscured by system UI.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  Size get physicalSize => _physicalSize;
  Size _physicalSize = Size.zero;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the application can render, but over which the operating system
  /// will likely place system UI, such as the keyboard, that fully obscures
  /// any content.
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  ///  * [Scaffold], which automatically applies the view insets in material
  ///    design applications.
  WindowPadding get viewInsets => _viewInsets;
  WindowPadding _viewInsets = WindowPadding.zero;

  /// The number of physical pixels on each side of the display rectangle into
  /// which the application can render, but which may be partially obscured by
  /// system UI (such as the system notification area), or or physical
  /// intrusions in the display (e.g. overscan regions on television screens or
  /// phone sensor housings).
  ///
  /// When this changes, [onMetricsChanged] is called.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  ///  * [Scaffold], which automatically applies the padding in material design
  ///    applications.
  WindowPadding get padding => _padding;
  WindowPadding _padding = WindowPadding.zero;

  /// A callback that is invoked whenever the [devicePixelRatio],
  /// [physicalSize], [padding], or [viewInsets] values change, for example
  /// when the device is rotated or when the application is resized (e.g. when
  /// showing applications side-by-side on Android).
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
  VoidCallback get onMetricsChanged => _onMetricsChanged;
  VoidCallback _onMetricsChanged;
  Zone _onMetricsChangedZone;
  set onMetricsChanged(VoidCallback callback) {
    _onMetricsChanged = callback;
    _onMetricsChangedZone = Zone.current;
  }

  /// The system-reported default locale of the device.
  ///
  /// This establishes the language and formatting conventions that application
  /// should, if possible, use to render their user interface.
  ///
  /// This is the first locale selected by the user and is the user's
  /// primary locale (the locale the device UI is displayed in)
  ///
  /// This is equivalent to `locales.first` and will provide an empty non-null locale
  /// if the [locales] list has not been set or is empty.
  Locale get locale {
    if (_locales != null && _locales.isNotEmpty) {
      return _locales.first;
    }
    return null;
  }

  /// The full system-reported supported locales of the device.
  ///
  /// This establishes the language and formatting conventions that application
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
  List<Locale> get locales => _locales;
  List<Locale> _locales;

  /// A callback that is invoked whenever [locale] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback get onLocaleChanged => _onLocaleChanged;
  VoidCallback _onLocaleChanged;
  Zone _onLocaleChangedZone;
  set onLocaleChanged(VoidCallback callback) {
    _onLocaleChanged = callback;
    _onLocaleChangedZone = Zone.current;
  }

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
  double get textScaleFactor => _textScaleFactor;
  double _textScaleFactor = 1.0;

  /// The setting indicating whether time should always be shown in the 24-hour
  /// format.
  ///
  /// This option is used by [showTimePicker].
  bool get alwaysUse24HourFormat => _alwaysUse24HourFormat;
  bool _alwaysUse24HourFormat = false;

  /// A callback that is invoked whenever [textScaleFactor] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  VoidCallback get onTextScaleFactorChanged => _onTextScaleFactorChanged;
  VoidCallback _onTextScaleFactorChanged;
  Zone _onTextScaleFactorChangedZone;
  set onTextScaleFactorChanged(VoidCallback callback) {
    _onTextScaleFactorChanged = callback;
    _onTextScaleFactorChangedZone = Zone.current;
  }

  /// A callback that is invoked to notify the application that it is an
  /// appropriate time to provide a scene using the [SceneBuilder] API and the
  /// [render] method. When possible, this is driven by the hardware VSync
  /// signal. This is only called if [scheduleFrame] has been called since the
  /// last time this callback was invoked.
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
  FrameCallback get onBeginFrame => _onBeginFrame;
  FrameCallback _onBeginFrame;
  Zone _onBeginFrameZone;
  set onBeginFrame(FrameCallback callback) {
    _onBeginFrame = callback;
    _onBeginFrameZone = Zone.current;
  }

  /// A callback that is invoked for each frame after [onBeginFrame] has
  /// completed and after the microtask queue has been drained. This can be
  /// used to implement a second phase of frame rendering that happens
  /// after any deferred work queued by the [onBeginFrame] phase.
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
  VoidCallback get onDrawFrame => _onDrawFrame;
  VoidCallback _onDrawFrame;
  Zone _onDrawFrameZone;
  set onDrawFrame(VoidCallback callback) {
    _onDrawFrame = callback;
    _onDrawFrameZone = Zone.current;
  }

  /// A callback that is invoked when pointer data is available.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [GestureBinding], the Flutter framework class which manages pointer
  ///    events.
  PointerDataPacketCallback get onPointerDataPacket => _onPointerDataPacket;
  PointerDataPacketCallback _onPointerDataPacket;
  Zone _onPointerDataPacketZone;
  set onPointerDataPacket(PointerDataPacketCallback callback) {
    _onPointerDataPacket = callback;
    _onPointerDataPacketZone = Zone.current;
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
  String get defaultRouteName => _defaultRouteName();
  String _defaultRouteName() native 'Window_defaultRouteName';

  /// Requests that, at the next appropriate opportunity, the [onBeginFrame]
  /// and [onDrawFrame] callbacks be invoked.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  void scheduleFrame() native 'Window_scheduleFrame';

  /// Updates the application's rendering on the GPU with the newly provided
  /// [Scene]. This function must be called within the scope of the
  /// [onBeginFrame] or [onDrawFrame] callbacks being invoked. If this function
  /// is called a second time during a single [onBeginFrame]/[onDrawFrame]
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
  void render(Scene scene) native 'Window_render';

  /// Whether the user has requested that [updateSemantics] be called when
  /// the semantic contents of window changes.
  ///
  /// The [onSemanticsEnabledChanged] callback is called whenever this value
  /// changes.
  bool get semanticsEnabled => _semanticsEnabled;
  bool _semanticsEnabled = false;

  /// A callback that is invoked when the value of [semanticsEnabled] changes.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  VoidCallback get onSemanticsEnabledChanged => _onSemanticsEnabledChanged;
  VoidCallback _onSemanticsEnabledChanged;
  Zone _onSemanticsEnabledChangedZone;
  set onSemanticsEnabledChanged(VoidCallback callback) {
    _onSemanticsEnabledChanged = callback;
    _onSemanticsEnabledChangedZone = Zone.current;
  }

  /// A callback that is invoked whenever the user requests an action to be
  /// performed.
  ///
  /// This callback is used when the user expresses the action they wish to
  /// perform based on the semantics supplied by [updateSemantics].
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  SemanticsActionCallback get onSemanticsAction => _onSemanticsAction;
  SemanticsActionCallback _onSemanticsAction;
  Zone _onSemanticsActionZone;
  set onSemanticsAction(SemanticsActionCallback callback) {
    _onSemanticsAction = callback;
    _onSemanticsActionZone = Zone.current;
  }

  /// Additional accessibility features that may be enabled by the platform.
  AccessibilityFeatures get accessibilityFeatures => _accessibilityFeatures;
  AccessibilityFeatures _accessibilityFeatures;

  /// A callback that is invoked when the value of [accessibilityFlags] changes.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  VoidCallback get onAccessibilityFeaturesChanged => _onAccessibilityFeaturesChanged;
  VoidCallback _onAccessibilityFeaturesChanged;
  Zone _onAccessibilityFlagsChangedZone;
  set onAccessibilityFeaturesChanged(VoidCallback callback) {
    _onAccessibilityFeaturesChanged = callback;
    _onAccessibilityFlagsChangedZone = Zone.current;
  }

  /// Change the retained semantics data about this window.
  ///
  /// If [semanticsEnabled] is true, the user has requested that this funciton
  /// be called whenever the semantic content of this window changes.
  ///
  /// In either case, this function disposes the given update, which means the
  /// semantics update cannot be used further.
  void updateSemantics(SemanticsUpdate update) native 'Window_updateSemantics';

  /// Set the debug name associated with this window's root isolate.
  ///
  /// Normally debug names are automatically generated from the Dart port, entry
  /// point, and source file. For example: `main.dart$main-1234`.
  ///
  /// This can be combined with flutter tools `--isolate-filter` flag to debug
  /// specific root isolates. For example: `flutter attach --isolate-filter=[name]`.
  /// Note that this does not rename any child isolates of the root.
  void setIsolateDebugName(String name) native 'Window_setIsolateDebugName';

  /// Sends a message to a platform-specific plugin.
  ///
  /// The `name` parameter determines which plugin receives the message. The
  /// `data` parameter contains the message payload and is typically UTF-8
  /// encoded JSON but can be arbitrary data. If the plugin replies to the
  /// message, `callback` will be called with the response.
  ///
  /// The framework invokes [callback] in the same zone in which this method
  /// was called.
  void sendPlatformMessage(String name,
                           ByteData data,
                           PlatformMessageResponseCallback callback) {
    final String error =
        _sendPlatformMessage(name, _zonedPlatformMessageResponseCallback(callback), data);
    if (error != null)
      throw new Exception(error);
  }
  String _sendPlatformMessage(String name,
                              PlatformMessageResponseCallback callback,
                              ByteData data) native 'Window_sendPlatformMessage';

  /// Called whenever this window receives a message from a platform-specific
  /// plugin.
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
  PlatformMessageCallback get onPlatformMessage => _onPlatformMessage;
  PlatformMessageCallback _onPlatformMessage;
  Zone _onPlatformMessageZone;
  set onPlatformMessage(PlatformMessageCallback callback) {
    _onPlatformMessage = callback;
    _onPlatformMessageZone = Zone.current;
  }

  /// Called by [_dispatchPlatformMessage].
  void _respondToPlatformMessage(int responseId, ByteData data)
      native 'Window_respondToPlatformMessage';

  /// Wraps the given [callback] in another callback that ensures that the
  /// original callback is called in the zone it was registered in.
  static PlatformMessageResponseCallback _zonedPlatformMessageResponseCallback(PlatformMessageResponseCallback callback) {
    if (callback == null)
      return null;

    // Store the zone in which the callback is being registered.
    final Zone registrationZone = Zone.current;

    return (ByteData data) {
      registrationZone.runUnaryGuarded(callback, data);
    };
  }
}

/// Additional accessibility features that may be enabled by the platform.
///
/// It is not possible to enable these settings from Flutter, instead they are
/// used by the platform to indicate that additional accessibility features are
/// enabled.
class AccessibilityFeatures {
  const AccessibilityFeatures._(this._index);

  static const int _kAccessibleNavigation = 1 << 0;
  static const int _kInvertColorsIndex = 1 << 1;
  static const int _kDisableAnimationsIndex = 1 << 2;
  static const int _kBoldTextIndex = 1 << 3;
  static const int _kReduceMotionIndex = 1 << 4;

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
    return 'AccessibilityFeatures$features';
  }

  @override
  bool operator ==(dynamic other) {
    if (other.runtimeType != runtimeType)
      return false;
    final AccessibilityFeatures typedOther = other;
    return _index == typedOther._index;
  }

  @override
  int get hashCode => _index.hashCode;
}

/// The [Window] singleton. This object exposes the size of the display, the
/// core scheduler API, the input event callback, the graphics drawing API, and
/// other such core services.
final Window window = new Window._();
