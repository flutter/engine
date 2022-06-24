// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show jsonDecode;
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:http/http.dart' as http;

import 'exceptions.dart';
import 'utils.dart';

class GenerateFallbackFontDataCommand extends Command<bool>
    with ArgUtils<bool> {
  GenerateFallbackFontDataCommand() {
    argParser.addOption(
      'key',
      defaultsTo: '',
      help: 'The Google Fonts API key. Used to get data about fonts hosted on '
          'Google Fonts',
    );
  }

  @override
  final String name = 'generate-fallback-font-data';

  @override
  final String description = 'Generate fallback font data from GoogleFonts';

  String get apiKey => stringArg('key');

  @override
  Future<bool> run() async {
    await _generateFallbackFontData();
    return true;
  }

  Future<void> _generateFallbackFontData() async {
    if (apiKey.isEmpty) {
      throw UsageException('No Google Fonts API key provided', argParser.usage);
    }
    final http.Client client = http.Client();
    final http.Response response = await client.get(Uri.parse(
        'https://www.googleapis.com/webfonts/v1/webfonts?key=$apiKey'));
    if (response.statusCode != 200) {
      throw ToolExit('Failed to download Google Fonts list.');
    }
    final Map<String, dynamic> googleFontsResult = jsonDecode(response.body);
    final List<dynamic> fontDatas = googleFontsResult['items'] as List<dynamic>;
    final Map<String, Uri> urlForFamily = <String, Uri>{};
    for (final Map<String, dynamic> fontData in fontDatas) {
      if (fallbackFonts.contains(fontData['family'])) {
        final Uri uri = Uri.parse(fontData['files']['regular'] as String);
        urlForFamily[fontData['family']] = uri;
      }
    }
    final io.Directory tempDir =
        await io.Directory.systemTemp.createTemp('fonts');
    for (final String family in fallbackFonts) {
      print('Downloading $family...');
      final http.Response fontResponse =
          await client.get(urlForFamily[family]!);
      if (fontResponse.statusCode != 200) {
        throw ToolExit('Failed to download font for $family');
      }
    }
  }
}

const List<String> fallbackFonts = <String>[
  'Noto Sans',
  'Noto Emoji',
  'Noto Sans Adlam',
  'Noto Sans Anatolian Hieroglyphs',
  'Noto Sans Arabic',
  'Noto Sans Armenian',
  'Noto Sans Avestan',
  'Noto Sans Balinese',
  'Noto Sans Bamum',
  'Noto Sans Bassa Vah',
  'Noto Sans Batak',
  'Noto Sans Bengali',
  'Noto Sans Bhaiksuki',
  'Noto Sans Brahmi',
  'Noto Sans Buginese',
  'Noto Sans Buhid',
  'Noto Sans Canadian Aboriginal',
  'Noto Sans Carian',
  'Noto Sans Caucasian Albanian',
  'Noto Sans Chakma',
  'Noto Sans Cham',
  'Noto Sans Cherokee',
  'Noto Sans Coptic',
  'Noto Sans Cuneiform',
  'Noto Sans Cypriot',
  'Noto Sans Deseret',
  'Noto Sans Devanagari',
  'Noto Sans Duployan',
  'Noto Sans Egyptian Hieroglyphs',
  'Noto Sans Elbasan',
  'Noto Sans Elymaic',
  'Noto Sans Georgian',
  'Noto Sans Glagolitic',
  'Noto Sans Gothic',
  'Noto Sans Grantha',
  'Noto Sans Gujarati',
  'Noto Sans Gunjala Gondi',
  'Noto Sans Gurmukhi',
  'Noto Sans HK',
  'Noto Sans Hanunoo',
  'Noto Sans Hatran',
  'Noto Sans Hebrew',
  'Noto Sans Imperial Aramaic',
  'Noto Sans Indic Siyaq Numbers',
  'Noto Sans Inscriptional Pahlavi',
  'Noto Sans Inscriptional Parthian',
  'Noto Sans JP',
  'Noto Sans Javanese',
  'Noto Sans KR',
  'Noto Sans Kaithi',
  'Noto Sans Kannada',
  'Noto Sans Kayah Li',
  'Noto Sans Kharoshthi',
  'Noto Sans Khmer',
  'Noto Sans Khojki',
  'Noto Sans Khudawadi',
  'Noto Sans Lao',
  'Noto Sans Lepcha',
  'Noto Sans Limbu',
  'Noto Sans Linear A',
  'Noto Sans Linear B',
  'Noto Sans Lisu',
  'Noto Sans Lycian',
  'Noto Sans Lydian',
  'Noto Sans Mahajani',
  'Noto Sans Malayalam',
  'Noto Sans Mandaic',
  'Noto Sans Manichean',
  'Noto Sans Marchen',
  'Noto Sans Masaram Gondi',
  'Noto Sans Math',
  'Noto Sans Mayan Numerals',
  'Noto Sans Medefaidrin',
  'Noto Sans Meetei Mayek',
  'Noto Sans Meroitic',
  'Noto Sans Miao',
  'Noto Sans Modi',
  'Noto Sans Mongolian',
  'Noto Sans Mro',
  'Noto Sans Multani',
  'Noto Sans Myanmar',
  'Noto Sans N Ko',
  'Noto Sans Nabataean',
  'Noto Sans New Tai Lue',
  'Noto Sans Newa',
  'Noto Sans Nushu',
  'Noto Sans Ogham',
  'Noto Sans Ol Chiki',
  'Noto Sans Old Hungarian',
  'Noto Sans Old Italic',
  'Noto Sans Old North Arabian',
  'Noto Sans Old Permic',
  'Noto Sans Old Persian',
  'Noto Sans Old Sogdian',
  'Noto Sans Old South Arabian',
  'Noto Sans Old Turkic',
  'Noto Sans Oriya',
  'Noto Sans Osage',
  'Noto Sans Osmanya',
  'Noto Sans Pahawh Hmong',
  'Noto Sans Palmyrene',
  'Noto Sans Pau Cin Hau',
  'Noto Sans Phags Pa',
  'Noto Sans Phoenician',
  'Noto Sans Psalter Pahlavi',
  'Noto Sans Rejang',
  'Noto Sans Runic',
  'Noto Sans SC',
  'Noto Sans Saurashtra',
  'Noto Sans Sharada',
  'Noto Sans Shavian',
  'Noto Sans Siddham',
  'Noto Sans Sinhala',
  'Noto Sans Sogdian',
  'Noto Sans Sora Sompeng',
  'Noto Sans Soyombo',
  'Noto Sans Sundanese',
  'Noto Sans Syloti Nagri',
  'Noto Sans Symbols',
  'Noto Sans Symbols 2',
  'Noto Sans Syriac',
  'Noto Sans TC',
  'Noto Sans Tagalog',
  'Noto Sans Tagbanwa',
  'Noto Sans Tai Le',
  'Noto Sans Tai Tham',
  'Noto Sans Tai Viet',
  'Noto Sans Takri',
  'Noto Sans Tamil',
  'Noto Sans Tamil Supplement',
  'Noto Sans Telugu',
  'Noto Sans Thaana',
  'Noto Sans Thai',
  'Noto Sans Tifinagh',
  'Noto Sans Tirhuta',
  'Noto Sans Ugaritic',
  'Noto Sans Vai',
  'Noto Sans Wancho',
  'Noto Sans Warang Citi',
  'Noto Sans Yi',
  'Noto Sans Zanabazar Square',
];
