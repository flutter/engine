// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('Locale', () {
    final Null $null = null;
    expect(
      const Locale('en').toString(),
      'en',
    );
    expect(
      const Locale('en'),
      new Locale('en', $null),
    );
    expect(
      const Locale('en').hashCode,
      new Locale('en', $null).hashCode,
    );

    expect(
      const Locale('en'),
      isNot(new Locale('en', '')),
      reason: 'Legacy. (The semantic difference between Locale("en") and '
          'Locale("en", "") is not defined.)',
    );
    expect(
      const Locale('en').hashCode,
      isNot(new Locale('en', '').hashCode),
      reason: 'Legacy. (The semantic difference between Locale("en") and '
          'Locale("en", "") is not defined.)',
    );
    expect(
      const Locale('en', 'US').toString(),
      'en_US',
      reason: 'Legacy. en_US is a valid Unicode Locale Identifier, but '
          'not a valid Unicode BCP47 Locale Identifier.',
    );
    expect(
      const Locale('en', 'US').toLanguageTag(),
      'en-US',
      reason: 'Unicode BCP47 Locale Identifier, as recommended for general '
          'interchange.',
    );

    expect(
      const Locale('iw').toString(),
      'he',
      reason: 'The language code for Hebrew was officially changed in 1989.',
    );
    expect(
      const Locale('iw', 'DD').toString(),
      'he_DE',
      reason: 'Legacy. This is a valid Unicode Locale Identifier, even if '
          'not a valid Unicode BCP47 Locale Identifier',
    );
    expect(
      const Locale('iw', 'DD'),
      const Locale('he', 'DE'),
      reason: 'The German Democratic Republic ceased to exist in '
          'October 1990.',
    );
  });

  test('Locale unnamed constructor idiosyncrasies', () {
    expect(
      () => Locale.parse(Locale('en', '').toString()),
      throwsException,
      reason: 'Locale("en", "").toString() produces "en-" which is not '
          'standards-compliant.',
    );

    // We have:
    expect(
      const Locale('en'),
      isNot(new Locale('en', '')),
      reason: 'Legacy. (The semantic difference between Locale("en") and '
          'Locale("en", "") is not defined.)',
    );
    // However we also have:
    expect(
      Locale.parse(Locale('en', '').toLanguageTag()),
      Locale('en'),
      reason: 'There is no standards-compliant way for toLanguageTag() to '
          'represent a zero-length region code.',
    );
    // So this class' operator== doesn't match the behaviour we expect from the
    // normalized Unicode BCP47 Locale Identifiers perspective.

    expect(
      Locale('abcd').toLanguageTag(),
      'abcd',
      reason: 'Locale("abcd") is not following instructions in the API '
          'documentation, so produces standards-uncompliant output.',
    );
    expect(
      Locale.parse('abcd').toLanguageTag(),
      'und-Abcd',
      reason: '',
    );
    expect(
      Locale('abcd'),
      isNot(Locale.parse('abcd')),
      reason: '',
    );
    expect(
      () => Locale.parse(Locale('a').toLanguageTag()),
      throwsException,
      reason: 'Locale("abcd") is not following instructions in the API '
          'documentation, so produces standards-uncompliant output.',
    );
    expect(
      Locale.parse(Locale('EN').toLanguageTag()).languageCode,
      'en',
      reason: 'Locale.parse does standards-compliant normalization, whereas '
          'Locale("EN") is incorrect usage of the API as per API '
          'documentation.',
    );

    // Syntax is correct. Without validating against CLDR supplemental
    // data, this looks like da-u-nu-true.
    expect(
      Locale.parse('da-u-nu').toLanguageTag(),
      equals('da-u-nu'),
      reason: 'da-u-nu syntax is correct, this looks like "da-u-nu-true". '
          'Only validation against CLDR validity data would show "true" is '
          'not a valid value for "nu".',
    );
  });

  group('Locale.parse():', () {
    test('languageCode.', () {
      expect(
        Locale.parse('IW').languageCode,
        'he',
        reason: "Case insensitive to input.",
      );
      expect(
        Locale.parse('Fil').languageCode,
        'fil',
        reason: "3-character language codes: Filipino is not Finnish.",
      );
      expect(
        Locale.parse('abcde').languageCode,
        'abcde',
        reason: 'The spec provides for language codes with 5 to 8 '
            'characters, though as of 2018, there aren\'t any valid tags '
            'this long.',
      );
      expect(
        Locale.parse('ROOT').languageCode,
        'und',
        reason: 'BCP 47 Language Tag Conversion: '
            'replace "root" with "und", and case insensitive.',
      );
    });

    test('scriptCode.', () {
      expect(
        Locale.parse('af_latn').scriptCode,
        'Latn',
      );
      expect(
        Locale.parse('zh_HANT-CN').scriptCode,
        'Hant',
      );
      expect(
        Locale.parse('sw-TZ').scriptCode,
        null,
      );
    });

    test('countryCode.', () {
      expect(
        Locale.parse('ar-Arab-EG').countryCode,
        'EG',
      );
      expect(
        Locale.parse('en-GB_scouse_fonipa').countryCode,
        'GB',
      );
    });

    test('variants.', () {
      // Liverpool English, script variant: International Phonetic Alphabet.
      // (fonipa is a Latn variant, so only Latn really makes sense as script)
      expect(
        Locale.parse('en_scouse_fonipa').variants,
        orderedEquals(['fonipa', 'scouse']),
        reason: 'Variants should be sorted alphabetically.',
      );
      expect(
        Locale.parse('de-1996').variants,
        orderedEquals(['1996']),
      );
      expect(
        Locale.parse('ja_Jpan').variants,
        orderedEquals([]),
        reason: 'No variants represented by zero-length Iterable.',
      );
    });

    test('Locale Identifiers with extensions.', () {
      expect(
        Locale.parse('nl-u-attr2-attr1').toLanguageTag(),
        equals('nl-u-attr2-attr1'),
        reason: '-u- attributes are ordered, do not sort.',
      );

      expect(
        Locale.parse('ar-u-ca-islamic-civil').toLanguageTag(),
        equals('ar-u-ca-islamic-civil'),
        reason: '-u- attributes are ordered, do not sort.',
      );

      expect(
        Locale.parse('RU-T-en-Cyrl-GB-H0-HYBRID').toLanguageTag(),
        equals('ru-t-en-cyrl-gb-h0-hybrid'),
        reason: 'Language identifiers in t-extensions are also lowercase.',
      );

      expect(
        Locale.parse('tr-x-foo-t-hi-h0-hybrid-u-nu').toLanguageTag(),
        equals('tr-x-foo-t-hi-h0-hybrid-u-nu'),
        reason: 'Everything after -x- belong to the -x- private use '
            'extension.',
      );

      expect(
        Locale.parse('pl-b-h0-hybrid-nu-roman').toLanguageTag(),
        equals('pl-b-h0-hybrid-nu-roman'),
        reason: 'What looks like tkey and key subtags appearing under '
            'singletons other than -t- and -u- belong to those singletons.',
      );

      expect(
        Locale.parse('ca-u-kb-true').toLanguageTag(),
        equals('ca-u-kb'),
        reason: 'For true/false keys like kb, true can be ommitted.',
      );
      expect(
        Locale.parse('ca-u-kb').toLanguageTag(),
        equals('ca-u-kb'),
        reason: 'For true/false keys like kb, true can be ommitted.',
      );
      // TODO: when adding accessor for u-kb: it should return 'true'.

      expect(
        Locale.parse('En-LATN-Gb-SCOUSE-FONIPA'
                '-U-ATTR2-ATTR1-ca-islamic-civil-nu-thai'
                '-A-BC'
                '-z-fo-xyzzy'
                '-T-HI-H0-hybrid-m0-UNGEGN'
                '-x-u-ab')
            .toLanguageTag(),
        equals('en-Latn-GB-fonipa-scouse'
            '-a-bc'
            '-t-hi-h0-hybrid-m0-ungegn'
            '-u-attr2-attr1-ca-islamic-civil-nu-thai'
            '-z-fo-xyzzy'
            '-x-u-ab'),
      );
    });

    // These examples are not spec compliant, although we could have been more
    // lenient.  When choosing to permit uncompliant identifiers, we would need
    // to decide "how lenient?", so we prefer to draw the most obvious line by
    // being strict.
    // TODO: maybe have the exception contain what's necessary to make a "best
    // effort" locale?
    test('Strict parsing examples.', () {
      expect(
        () => Locale.parse('nl-u-x'), throws,
        reason: 'With no "u" attributes there should be no "u" singleton. '
            'Could be lenient: "nl-x".',
      );
      expect(
        () => Locale.parse('fr-t-x'), throws,
        reason: 'With no "t" attributes there should be no "t" singleton. '
            'Could be lenient: "fr-x".',
      );
      expect(
        () => Locale.parse('it-u-nu-romanlow-a-u-attr'), throws,
        reason: 'Duplicate "u" singletons could be merged if only one has '
            'attributes. Could be lenient: "it-a-u-attr-nu-romanlow".',
      );
      expect(
        () => Locale.parse('pl-t-cs-b-t-h0-hybrid'), throws,
        reason: 'Duplicate "t" singletons could be merged if only one has '
            'tlang specified. Could be lenient: "pl-b-t-cs-h0-hybrid".',
      );

      expect(
        () => Locale.parse('ro-t-hu-HU-cu-ron'), throws,
        reason: 'U-extension keywords misplaced under the -t- singleton '
            'could be moved if unambiguous enough. '
            'Could be lenient: "ro-t-hu-hu-u-cu-ron".',
      );
      // TODO: any point to this test? It's a counter-example of the previous
      // "lenient parsing" idea.
      expect(
        Locale.parse('ro-t-nu-cyrl').toLanguageTag(),
        equals('ro-t-nu-cyrl'),
        reason: 'This cannot be interpreted as a misplaced -u- keyword. '
            'It looks like a tlang tag: "nu-Cyrl", ',
      );

      expect(
        () => Locale.parse('pt-BR-u-h0-hybrid-t-pt-PT'), throws,
        reason: 'T-extension "tfields" misplaced under the U-extension. '
            'Could be lenient: "pt-BR-t-pt-pt-h0-hybrid".',
      );

      expect(
        () => Locale.parse('pl-t-h0'), throws,
        reason: 'Locale tag pl-t-h0 is not spec compliant. How to best fix it '
            'is unclear: it is underspecified.',
      );
    });

    test('Locale.parse(): invalid identifiers.', () {
      expect(
        () => Locale.parse('a'), throws,
        reason: 'One character language subtags are invalid.',
      );
      expect(
        Locale.parse('abcd').languageCode,
        'und',
        reason: 'Special-use corner case from the specification: '
            'language subtag can be skipped if a script is specified.',
      );
      expect(
        Locale.parse('abcd').scriptCode,
        'Abcd',
        reason: 'Special-use corner case from the specification: '
            'language subtag can be skipped if a script is specified.',
      );
      expect(
        () => Locale.parse('abcdefghi'), throws,
        reason: 'Language subtags may not be more than 8 characters.',
      );
      expect(
        () => Locale.parse(r'e$'), throws,
        reason: 'Invalid character for language subtag, only a-z allowed.',
      );
      expect(
        () => Locale.parse('fr-RU-Hant'), throws,
        reason: 'Swapping region and script is not allowed.',
      );
    });
  });

  test('Equality.', () {
    expect(
      Locale.parse('en'),
      isNot(Locale.parse('en-Latn')),
    );
    expect(
      Locale.parse('en').hashCode,
      isNot(Locale.parse('en-Latn').hashCode),
    );

    expect(
      Locale.parse('en'),
      isNot(Locale.parse('en-US')),
    );
    expect(
      Locale.parse('en').hashCode,
      isNot(Locale.parse('en-US').hashCode),
    );

    expect(
      Locale.parse('en'),
      isNot(Locale.parse('en-fonipa')),
    );
    expect(
      Locale.parse('en').hashCode,
      isNot(Locale.parse('en-fonipa').hashCode),
    );

    expect(Locale.parse('en'), isNot(Locale.parse('en-a')));
    expect(
      Locale.parse('en').hashCode,
      isNot(Locale.parse('en-a').hashCode),
    );

    expect(Locale.parse('en'), isNot(Locale.parse('en-a')));
    expect(
      Locale.parse('en').hashCode,
      isNot(Locale.parse('en-a').hashCode),
    );

    expect(
      Locale.parse('en-u-attr'),
      isNot(Locale.parse('en-u-nu-roman')),
    );
    expect(
      Locale.parse('en-u-attr').hashCode,
      isNot(Locale.parse('en-u-nu-roman').hashCode),
    );

    Locale a = Locale.parse('en-u-kb');
    Locale b = Locale.parse('en-u-kb-true');
    expect(
      a,
      b,
      reason: '-u-kb should parse to the same result as -u-kb-true.',
    );
    expect(
      a.hashCode,
      b.hashCode,
      reason: '-u-kb should parse to the same result as -u-kb-true. ${a.myexthelper}, ${b.myexthelper}.',
    );

    expect(
      Locale.parse('en-t-hi'),
      isNot(Locale.parse('en-t-hi-h0-hybrid')),
    );
    expect(
      Locale.parse('en-t-hi').hashCode,
      isNot(Locale.parse('en-t-hi-h0-hybrid').hashCode),
    );
  });
}
