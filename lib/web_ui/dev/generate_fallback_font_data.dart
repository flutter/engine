// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show jsonDecode;

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
    for (final Map<String, dynamic> fontData in fontDatas) {
      print(fontData['family']);
    }
  }
}

const List<String> fallbackFonts = <String>[
  'Noto Sans',
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
  // TODO(het): add more
  'Noto Emoji',
];
