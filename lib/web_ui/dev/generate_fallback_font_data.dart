// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show jsonDecode;
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:http/http.dart' as http;
import 'package:path/path.dart' as path;

import 'environment.dart';
import 'exceptions.dart';
import 'utils.dart';

// This is needed when making the request for the CSS font to get the split font
// declaration.
const String kBrowserUserAgent =
    'Mozilla/5.0 AppleWebKit/537.36 Chrome/111.0.0.0';

final http.Client client = http.Client();

class GenerateFallbackFontDataCommand extends Command<bool>
    with ArgUtils<bool> {
  GenerateFallbackFontDataCommand() {
    argParser.addOption(
      'key',
      defaultsTo: '',
      help: 'The Google Fonts API key. Used to get data about fonts hosted on '
          'Google Fonts.',
    );
    argParser.addFlag(
      'download-test-fonts',
      defaultsTo: true,
      help: 'Whether to download the Noto fonts into a local folder to use in'
          'tests.',
    );
  }

  @override
  final String name = 'generate-fallback-font-data';

  @override
  final String description = 'Generate fallback font data from GoogleFonts';

  String get apiKey => stringArg('key');

  bool get downloadTestFonts => boolArg('download-test-fonts');

  @override
  Future<bool> run() async {
    await _generateFallbackFontData();
    return true;
  }

  Future<void> _generateFallbackFontData() async {
    if (apiKey.isEmpty) {
      throw UsageException('No Google Fonts API key provided', argParser.usage);
    }
    final http.Response response = await client.get(Uri.parse(
        'https://www.googleapis.com/webfonts/v1/webfonts?key=$apiKey'));
    if (response.statusCode != 200) {
      throw ToolExit('Failed to download Google Fonts list.');
    }
    final Map<String, dynamic> googleFontsResponse =
        jsonDecode(response.body) as GoogleFontsResponse;
    final Map<String, FontResponse> fontItems = googleFontsResponse.itemsMap;

    // Check that each font family we care about is available in Google Fonts.
    for (final String family in fallbackFonts) {
      if (!fontItems.containsKey(family)) {
        throw ToolExit('Unable to determine URL to download $family. '
            'Check if it is still hosted on Google Fonts.');
      }
    }

    final io.Directory fontDir = downloadTestFonts
        ? await io.Directory(path.join(
            environment.webUiBuildDir.path,
            'assets',
            'noto',
          )).create(recursive: true)
        : await io.Directory.systemTemp.createTemp('fonts');

    // Delete old fonts in the font directory.
    await for (final io.FileSystemEntity file in fontDir.list()) {
      await file.delete();
    }

    final List<FontData> fontDatas = <FontData>[];
    for (final FontResponse fontResponse in fontItems.values) {
      if (cssFonts.contains(fontResponse.family)) {
        fontDatas.addAll(await extractFontDatasForCssFont(
          fontResponse: fontResponse,
          fontDir: fontDir,
        ));
      } else {
        fontDatas.add(await extractFontDataUsingFcQuery(
          fontResponse: fontResponse,
          fontDir: fontDir,
        ));
      }
    }

    final StringBuffer sb = StringBuffer();

    sb.writeln('// Copyright 2013 The Flutter Authors. All rights reserved.');
    sb.writeln('// Use of this source code is governed by a BSD-style license '
        'that can be');
    sb.writeln('// found in the LICENSE file.');
    sb.writeln();
    sb.writeln('// DO NOT EDIT! This file is generated. See:');
    sb.writeln('// dev/generate_fallback_font_data.dart');
    sb.writeln("import '../configuration.dart';");
    sb.writeln("import 'noto_font.dart';");
    sb.writeln();

    sb.writeln('final List<NotoFont> fallbackFonts = <NotoFont>[');
    for (final FontData fontData in fontDatas) {
      if (fontData.family == 'Noto Emoji') {
        sb.write(' if (!configuration.useColorEmoji)');
      }
      if (fontData.family == 'Noto Color Emoji') {
        sb.write(' if (configuration.useColorEmoji)');
      }
      sb.writeln(" NotoFont('${fontData.family}', '${fontData.uri}',");
      // Print the unicode ranges in a readable format for easier review. This
      // shouldn't affect code size because comments are removed in release mode.
      sb.write('   // <int>[');
      for (final String start in fontData.starts) {
        sb.write('0x$start,');
      }
      sb.writeln('],');
      sb.write('   // <int>[');
      for (final String end in fontData.ends) {
        sb.write('0x$end,');
      }
      sb.writeln(']');

      sb.writeln("   '${_packFontRanges(fontData.starts, fontData.ends)}',");
      sb.writeln(' ),');
    }
    sb.writeln('];');

    final io.File fontDataFile = io.File(path.join(
      environment.webUiRootDir.path,
      'lib',
      'src',
      'engine',
      'canvaskit',
      'font_fallback_data.dart',
    ));
    await fontDataFile.writeAsString(sb.toString());
  }
}

typedef GoogleFontsResponse = Map<String, dynamic>;
extension GoogleFontsResponseExtension on GoogleFontsResponse {
  Iterable<FontResponse> get items => (this['items'] as List<dynamic>).cast<FontResponse>();

  /// Maps each font family to its corresponding [FontResponse].
  Map<String, FontResponse> get itemsMap {
    final Map<String, FontResponse> map = <String, FontResponse>{};
    for (final String family in fallbackFonts) {
      final FontResponse? fontResponse = items.getFontResponseForFamily(family);
      if (fontResponse != null) {
        map[family] = fontResponse;
      }
    }
    return map;
  }
}

extension on Iterable<FontResponse> {
  FontResponse? getFontResponseForFamily(String family) {
    for (final FontResponse fontResponse in this) {
      if (fontResponse.family == family) {
        return fontResponse;
      }
    }
    return null;
  }
}

typedef FontResponse = Map<String, dynamic>;
extension FontResponseExtension on FontResponse {
  String get family => this['family'] as String;
  Iterable<String> get variants => (this['variants'] as List<dynamic>).cast<String>();
  Iterable<String> get subsets => (this['subsets'] as List<dynamic>).cast<String>();
  String get version => this['version'] as String;
  String get lastModified => this['lastModified'] as String;
  Map<String, String> get files => (this['files'] as Map<dynamic, dynamic>).cast<String, String>();
  String get category => this['category'] as String;
  String get kind => this['kind'] as String;

  String get cssFontUri => 'https://fonts.googleapis.com/css?family=${Uri.encodeComponent(family)}';
}

typedef FontData = ({
  String family,
  Uri uri,
  List<String> starts,
  List<String> ends,
});

typedef UnicodeRanges = Iterable<String>;
extension on UnicodeRanges {
  (List<String> starts, List<String> ends) toStartsAndEnds() {
    final List<String> starts = <String>[];
    final List<String> ends = <String>[];
    for (final String range in this) {
      final List<String> parts = range.split('-');
      switch (parts) {
        case [final String start, final String end]:
          starts.add(start);
          ends.add(end);
        case [final String start]:
          starts.add(start);
          ends.add(start);
      }
    }
    return (starts, ends);
  }
}

typedef CssString = String;
extension on CssString {
  Iterable<String> getFontFaces() {
    final RegExp fontFaceRegex =
        RegExp(r'@font-face\s*{.+?}', multiLine: true, dotAll: true);
    return fontFaceRegex.allMatches(this).map((Match match) => match.group(0)!);
  }
}

typedef FontFaceString = String;
extension on FontFaceString {
  Uri getSrcUri() {
    final RegExp srcRegex = RegExp(r'src:\s*url\((.+?)\)');
    final Match? match = srcRegex.firstMatch(this);
    if (match == null) {
      throw ToolExit('Failed to find src in font face:\n$this');
    }
    return Uri.parse(match.group(1)!);
  }

  UnicodeRanges getUnicodeRanges() {
    final RegExp unicodeRangeRegex = RegExp(r'unicode-range:(.+?);');
    final Match? match = unicodeRangeRegex.firstMatch(this);
    if (match == null) {
      throw ToolExit('Failed to find unicode-range in font face:\n$this');
    }
    return match
        .group(1)!
        .split(',')
        .map((String item) => item.replaceFirst('U+', '').trim());
  }
}

final RegExp cssUnicodeRange = RegExp(r'unicode-range:\s*U\+([0-9A-F]+)-U\+([0-9A-F]+)');
Future<List<FontData>> extractFontDatasForCssFont({
  required FontResponse fontResponse,
  required io.Directory fontDir,
}) async {
  final CssString css = await downloadCssForFont(fontResponse);

  final List<FontData> fontDatas = <FontData>[];
  final Iterable<String> fontFaces = css.getFontFaces();
  // We expect at least 3 font faces to make sure that we got the split version
  // of the font.
  assert(fontFaces.length >= 3, 'Expected at least 3 font faces, but found only ${fontFaces.length}.');

  for (final String fontFace in fontFaces) {
    final Iterable<String> unicodeRanges = fontFace.getUnicodeRanges();
    final (List<String> starts, List<String> ends) =
        unicodeRanges.toStartsAndEnds();

    fontDatas.add((
      family: fontResponse.family,
      uri: fontFace.getSrcUri(),
      starts: starts,
      ends: ends,
    ));
  }
  return fontDatas;
}

Future<CssString> downloadCssForFont(FontResponse fontResponse) async {
  final Uri cssFontUri = Uri.parse(fontResponse.cssFontUri);
  print('Downloading css for font ${fontResponse.family}...');
  final http.Response httpResponse = await client.get(
    cssFontUri,
    headers: <String, String>{'User-Agent': kBrowserUserAgent},
  );
  if (httpResponse.statusCode != 200) {
    throw ToolExit(
      'Failed to download css for font "${fontResponse.family}" (uri: $cssFontUri)',
    );
  }
  final CssString css = httpResponse.body;
  if (!css.contains('unicode-range:')) {
    throw ToolExit(
      'Failed to find unicode-range in css for font "${fontResponse.family}". '
      'Try adjusting http headers to fool the server into thinking we are a '
      'browser.',
    );
  }
  return css;
}

Future<FontData> extractFontDataUsingFcQuery({
  required FontResponse fontResponse,
  required io.Directory fontDir,
}) async {
  // 1. Download the font file.
  final Uri fontUri = Uri.parse(fontResponse.files['regular']!).replace(scheme: 'https');
  final io.File fontFile = await downloadFont(
    family: fontResponse.family,
    uri: fontUri,
    fontDir: fontDir,
  );

  // 2. Run fc-query to get the list of unicode ranges.
  final io.ProcessResult fcQueryResult = await io.Process.run(
    'fc-query',
    <String>[
      '--format=%{charset}',
      '--',
      fontFile.path,
    ],
  );
  final String encodedCharset = fcQueryResult.stdout as String;
  final UnicodeRanges unicodeRanges = encodedCharset.split(' ');
  final (List<String> starts, List<String> ends) = unicodeRanges.toStartsAndEnds();

  // 3. Create the font data.
  return (
    family: fontResponse.family,
    uri: fontUri,
    starts: starts,
    ends: ends,
  );
}

Future<io.File> downloadFont({
  required String family,
  required Uri uri,
  required io.Directory fontDir,
}) async {
  print('Downloading $family...');
  final http.Response httpResponse = await client.get(uri);
  if (httpResponse.statusCode != 200) {
    throw ToolExit('Failed to download font for "$family" (uri: $uri)');
  }
  final io.File fontFile =
      io.File(path.join(fontDir.path, path.basename(uri.path)));
  await fontFile.writeAsBytes(httpResponse.bodyBytes);
  return fontFile;
}

const List<String> cssFonts = <String>[
  'Noto Color Emoji',
];

const List<String> fallbackFonts = <String>[
  'Noto Sans',
  'Noto Color Emoji',
  'Noto Emoji',
  'Noto Sans Symbols',
  'Noto Sans Symbols 2',
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
  'Noto Sans Manichaean',
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
  'Noto Sans NKo',
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

String _packFontRanges(List<String> starts, List<String> ends) {
  assert(starts.length == ends.length);

  final StringBuffer sb = StringBuffer();

  for (int i = 0; i < starts.length; i++) {
    final int start = int.parse(starts[i], radix: 16);
    final int end = int.parse(ends[i], radix: 16);

    sb.write(start.toRadixString(36));
    sb.write('|');
    if (start != end) {
      sb.write((end - start).toRadixString(36));
    }
    sb.write(';');
  }

  return sb.toString();
}
