

import 'dart:typed_data';

import 'assets.dart';

abstract class FontCollection {
  Future<void> loadFontFromList(Uint8List list, {String? fontFamily});
  Future<void> ensureFontsLoaded();
  Future<void> registerFonts(AssetManager assetManager);
  void debugRegisterTestFonts();
  void clear();
}
