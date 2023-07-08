// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

// The country codes that use Hant (chinese traditional) script
const Set<String> _hantCountryCodes = <String>{'TW', 'HK', 'MO'};

/// Converts a list of IETF language tags (as Strings) into a List of [ui.Locale].
///
/// This method is a port of another one found in Flutter, here:
/// * https://github.com/flutter/flutter/blob/2b6492426e5c9a17f3597f40323ce902d4540396/dev/tools/localization/localizations_utils.dart#L39
///
/// Attempts to follow https://www.rfc-editor.org/info/bcp47, but this method
/// only covers very straightforward tags (up to 3 subtags).
///
/// This function has a special case for the Chinese language ('zh') that can
/// have different "scripts" based on the country code.
///
// TODO(dit): Reimplement this with Intl.Locale, https://github.com/flutter/flutter/issues/130174
// See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Locale/Locale
List<ui.Locale> parseLanguages(List<String> languages) {
  final List<ui.Locale> locales = <ui.Locale>[];

  for (final String language in languages) {
    final List<String> parts = language.split('-');
    ui.Locale? locale;
    if (parts.isNotEmpty) {
      // The first part tends to be the language code.
      final String languageCode = parts.first;
      assert(languageCode.trim().isNotEmpty, '"$language" is not a valid IETF language tag.');

      // Attempt to determine country and script from the next parts (if present).
      String? country;
      String? script;
      if (parts.length == 2) {
        // Country codes have length < 4. For example: 'en-US'
        // Script codes have length >= 4. For example: 'zh-Hant'
        (country, script) =
            parts[1].length < 4 ? (parts[1], null) : (null, parts[1]);
      } else if (parts.length == 3) {
        // Country codes are shorter than Script codes, so this code lets us support:
        // 'en-GB-oxendict' ("Oxford spelling") and 'zh-Hant-TW'
        //
        // Note that `oxendict` is a Variant, not a Script (but ui.Locale does
        // not support IETF variants). The language would be slightly mis-identified
        // this way, but parsing of country is still valid.
        //
        // See "Type: variant" here: https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry,
        (country, script) = parts[1].length < parts[2].length
            ? (parts[1], parts[2])
            : (parts[2], parts[1]);
      }

      // For Chinese, if the script is not found, compute from the country.
      if (languageCode == 'zh' && script == null) {
        script = _hantCountryCodes.contains(country) ? 'Hant' : 'Hans';
      }

      locale = ui.Locale.fromSubtags(
        languageCode: languageCode,
        countryCode: country,
        scriptCode: script,
      );
    }

    if (locale != null) {
      locales.add(locale);
    }
  }

  return locales;
}
