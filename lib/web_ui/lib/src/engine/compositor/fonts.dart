// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

// This URL was found by using the Google Fonts Developer API to find the URL
// for Roboto. The API warns that this URL is not stable. In order to update
// this, list out all of the fonts and find the URL for the regular
// Roboto font. The API reference is here:
// https://developers.google.com/fonts/docs/developer_api
const String _robotoUrl =
    'https://fonts.gstatic.com/s/roboto/v20/KFOmCnqEu92Fr1Me5WZLCzYlKw.ttf';

/// Manages the fonts used in the Skia-based backend.
class SkiaFontCollection {
  final List<Future<_RegisteredFont>> _unloadedAssetFonts =
      <Future<_RegisteredFont>>[];
  final List<_RegisteredFont> _assetFonts = <_RegisteredFont>[];
  final List<_RegisteredFont> _dynamicallyLoadedFonts = <_RegisteredFont>[];

  /// A mapping from the name a font was registered with, to the family name
  /// embedded in the font's bytes (the font's "actual" name).
  ///
  /// For example, a font may be registered in Flutter assets with the name
  /// "MaterialIcons", but if you read the family name out of the font's bytes
  /// it is actually "Material Icons". Skia works with the actual names of the
  /// fonts, so when we create a Skia Paragraph with Flutter font families, we
  /// must convert them to their actual family name when we pass them to Skia.
  final Map<String, String> fontFamilyOverrides = <String, String>{};

  final Set<String> registeredFamilies = <String>{};

  Future<void> ensureFontsLoaded() async {
    if (_unloadedAssetFonts.isNotEmpty) {
      _assetFonts.addAll(
          (await Future.wait(_unloadedAssetFonts)).where((x) => x != null));
      _unloadedAssetFonts.clear();
    }
    List<_RegisteredFont> registeredFonts = <_RegisteredFont>[
      ..._assetFonts,
      ..._dynamicallyLoadedFonts,
    ];

    fontFamilyOverrides.clear();

    for (_RegisteredFont font in registeredFonts) {
      if (fontFamilyOverrides.containsKey(font.flutterFamily)) {
        if (fontFamilyOverrides[font.flutterFamily] != font.actualFamily) {
          html.window.console.warn('Fonts in family ${font.flutterFamily} '
              'have different actual family names.');
          html.window.console.warn(
              'Current actual family: ${fontFamilyOverrides[font.flutterFamily]}');
          html.window.console.warn('New actual family: ${font.actualFamily}');
        }
      } else {
        fontFamilyOverrides[font.flutterFamily] = font.actualFamily;
      }
    }

    final List<Uint8List> fontBuffers =
        registeredFonts.map<Uint8List>((f) => f.bytes).toList();

    skFontMgr = canvasKit['SkFontMgr'].callMethod('FromData', fontBuffers);
  }

  Future<void> loadFontFromList(Uint8List list, {String fontFamily}) async {
    String actualFamily = _readActualFamilyName(list);

    if (actualFamily == null) {
      if (fontFamily == null) {
        html.window.console
            .warn('Failed to read font family name. Aborting font load.');
        return;
      }
      actualFamily = fontFamily;
    }

    if (fontFamily == null) {
      fontFamily = actualFamily;
    }

    registeredFamilies.add(fontFamily);

    _dynamicallyLoadedFonts
        .add(_RegisteredFont(list, fontFamily, actualFamily));
    await ensureFontsLoaded();
  }

  Future<void> registerFonts(AssetManager assetManager) async {
    ByteData byteData;

    try {
      byteData = await assetManager.load('FontManifest.json');
    } on AssetManagerException catch (e) {
      if (e.httpStatus == 404) {
        html.window.console
            .warn('Font manifest does not exist at `${e.url}` – ignoring.');
        return;
      } else {
        rethrow;
      }
    }

    if (byteData == null) {
      throw AssertionError(
          'There was a problem trying to load FontManifest.json');
    }

    final List<dynamic> fontManifest =
        json.decode(utf8.decode(byteData.buffer.asUint8List()));
    if (fontManifest == null) {
      throw AssertionError(
          'There was a problem trying to load FontManifest.json');
    }

    for (Map<String, dynamic> fontFamily in fontManifest) {
      final String family = fontFamily['family'];
      final List<dynamic> fontAssets = fontFamily['fonts'];

      registeredFamilies.add(family);

      for (dynamic fontAssetItem in fontAssets) {
        final Map<String, dynamic> fontAsset = fontAssetItem;
        final String asset = fontAsset['asset'];
        _unloadedAssetFonts
            .add(_registerFont(assetManager.getAssetUrl(asset), family));
      }
    }

    /// We need a default fallback font for CanvasKit, in order to
    /// avoid crashing while laying out text with an unregistered font. We chose
    /// Roboto to match Android.
    if (!registeredFamilies.contains('Roboto')) {
      // Download Roboto and add it to the font buffers.
      _unloadedAssetFonts.add(_registerFont(_robotoUrl, 'Roboto'));
    }
  }

  Future<_RegisteredFont> _registerFont(String url, String family) async {
    ByteBuffer buffer;
    try {
      buffer = await html.window.fetch(url).then(_getArrayBuffer);
    } catch (e) {
      html.window.console.warn('Failed to load font $family at $url');
      html.window.console.warn(e);
      return null;
    }

    final Uint8List bytes = buffer.asUint8List();
    String actualFamily = _readActualFamilyName(bytes);

    if (actualFamily == null) {
      html.window.console.warn('Failed to determine the actual name of the '
          'font $family at $url. Defaulting to $family.');
      actualFamily = family;
    }

    return _RegisteredFont(bytes, family, actualFamily);
  }

  String _readActualFamilyName(Uint8List bytes) {
    final js.JsObject tmpFontMgr =
        canvasKit['SkFontMgr'].callMethod('FromData', <Uint8List>[bytes]);
    String actualFamily = tmpFontMgr.callMethod('getFamilyName', <int>[0]);
    return actualFamily;
  }

  Future<ByteBuffer> _getArrayBuffer(dynamic fetchResult) {
    return fetchResult.arrayBuffer().then<ByteBuffer>((x) => x as ByteBuffer);
  }

  js.JsObject skFontMgr;
}

/// Represents a font that has been registered.
class _RegisteredFont {
  /// The font family that the font was declared to have by Flutter.
  final String flutterFamily;

  /// The byte data for this font.
  final Uint8List bytes;

  /// The font family that was parsed from the font's bytes.
  final String actualFamily;

  _RegisteredFont(this.bytes, this.flutterFamily, this.actualFamily)
      : assert(bytes != null),
        assert(flutterFamily != null),
        assert(actualFamily != null);
}
