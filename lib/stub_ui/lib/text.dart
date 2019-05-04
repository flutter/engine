// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

/// Stub implementation. See docs in `../ui/`.
enum FontStyle {
  /// Stub implementation. See docs in `../ui/`.
  normal,

  /// Stub implementation. See docs in `../ui/`.
  italic,
}

/// Stub implementation. See docs in `../ui/`.
class FontWeight {
  const FontWeight._(this.index);

  /// Stub implementation. See docs in `../ui/`.
  final int index;

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w100 = const FontWeight._(0);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w200 = const FontWeight._(1);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w300 = const FontWeight._(2);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w400 = const FontWeight._(3);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w500 = const FontWeight._(4);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w600 = const FontWeight._(5);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w700 = const FontWeight._(6);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w800 = const FontWeight._(7);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight w900 = const FontWeight._(8);

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight normal = w400;

  /// Stub implementation. See docs in `../ui/`.
  static const FontWeight bold = w700;

  /// Stub implementation. See docs in `../ui/`.
  static const List<FontWeight> values = const <FontWeight>[
    w100, w200, w300, w400, w500, w600, w700, w800, w900
  ];

  /// Stub implementation. See docs in `../ui/`.
  static FontWeight lerp(FontWeight a, FontWeight b, double t) {
    assert(t != null);
    return values[lerpDouble(a?.index ?? normal.index, b?.index ?? normal.index, t).round().clamp(0, 8)];
  }

  @override
  String toString() {
    return const <int, String>{
      0: 'FontWeight.w100',
      1: 'FontWeight.w200',
      2: 'FontWeight.w300',
      3: 'FontWeight.w400',
      4: 'FontWeight.w500',
      5: 'FontWeight.w600',
      6: 'FontWeight.w700',
      7: 'FontWeight.w800',
      8: 'FontWeight.w900',
    }[index];
  }
}

/// Stub implementation. See docs in `../ui/`.
// The order of this enum must match the order of the values in RenderStyleConstants.h's ETextAlign.
enum TextAlign {
  /// Stub implementation. See docs in `../ui/`.
  left,

  /// Stub implementation. See docs in `../ui/`.
  right,

  /// Stub implementation. See docs in `../ui/`.
  center,

  /// Stub implementation. See docs in `../ui/`.
  justify,

  /// Stub implementation. See docs in `../ui/`.
  start,

  /// Stub implementation. See docs in `../ui/`.
  end,
}

/// Stub implementation. See docs in `../ui/`.
enum TextBaseline {
  /// Stub implementation. See docs in `../ui/`.
  alphabetic,

  /// Stub implementation. See docs in `../ui/`.
  ideographic,
}

/// Stub implementation. See docs in `../ui/`.
class TextDecoration {
  const TextDecoration._(this._mask);

  /// Stub implementation. See docs in `../ui/`.
  factory TextDecoration.combine(List<TextDecoration> decorations) {
    int mask = 0;
    for (TextDecoration decoration in decorations)
      mask |= decoration._mask;
    return new TextDecoration._(mask);
  }

  final int _mask;

  /// Stub implementation. See docs in `../ui/`.
  bool contains(TextDecoration other) {
    return (_mask | other._mask) == _mask;
  }

  /// Stub implementation. See docs in `../ui/`.
  static const TextDecoration none = const TextDecoration._(0x0);

  /// Stub implementation. See docs in `../ui/`.
  static const TextDecoration underline = const TextDecoration._(0x1);

  /// Stub implementation. See docs in `../ui/`.
  static const TextDecoration overline = const TextDecoration._(0x2);

  /// Stub implementation. See docs in `../ui/`.
  static const TextDecoration lineThrough = const TextDecoration._(0x4);

  @override
  bool operator ==(dynamic other) {
    if (other is! TextDecoration)
      return false;
    final TextDecoration typedOther = other;
    return _mask == typedOther._mask;
  }

  @override
  int get hashCode => _mask.hashCode;

  @override
  String toString() {
    if (_mask == 0)
      return 'TextDecoration.none';
    final List<String> values = <String>[];
    if (_mask & underline._mask != 0)
      values.add('underline');
    if (_mask & overline._mask != 0)
      values.add('overline');
    if (_mask & lineThrough._mask != 0)
      values.add('lineThrough');
    if (values.length == 1)
      return 'TextDecoration.${values[0]}';
    return 'TextDecoration.combine([${values.join(", ")}])';
  }
}

/// Stub implementation. See docs in `../ui/`.
enum TextDecorationStyle {
  /// Stub implementation. See docs in `../ui/`.
  solid,

  /// Stub implementation. See docs in `../ui/`.
  double,

  /// Stub implementation. See docs in `../ui/`.
  dotted,

  /// Stub implementation. See docs in `../ui/`.
  dashed,

  /// Stub implementation. See docs in `../ui/`.
  wavy
}

/// Stub implementation. See docs in `../ui/`.
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

// This encoding must match the C++ version of ParagraphBuilder::pushStyle.
//
// The encoded array buffer has 8 elements.
//
//  - Element 0: A bit field where the ith bit indicates wheter the ith element
//    has a non-null value. Bits 8 to 12 indicate whether |fontFamily|,
//    |fontSize|, |letterSpacing|, |wordSpacing|, and |height| are non-null,
//    respectively. Bit 0 is unused.
//
//  - Element 1: The |color| in ARGB with 8 bits per channel.
//
//  - Element 2: A bit field indicating which text decorations are present in
//    the |textDecoration| list. The ith bit is set if there's a TextDecoration
//    with enum index i in the list.
//
//  - Element 3: The |decorationColor| in ARGB with 8 bits per channel.
//
//  - Element 4: The bit field of the |decorationStyle|.
//
//  - Element 5: The index of the |fontWeight|.
//
//  - Element 6: The enum index of the |fontStyle|.
//
//  - Element 7: The enum index of the |textBaseline|.
//
Int32List _encodeTextStyle(
  Color color,
  TextDecoration decoration,
  Color decorationColor,
  TextDecorationStyle decorationStyle,
  double decorationThickness,
  FontWeight fontWeight,
  FontStyle fontStyle,
  TextBaseline textBaseline,
  String fontFamily,
  List<String> fontFamilyFallback,
  double fontSize,
  double letterSpacing,
  double wordSpacing,
  double height,
  Locale locale,
  Paint background,
  Paint foreground,
  List<Shadow> shadows
) {
  final Int32List result = new Int32List(8);
  if (color != null) {
    result[0] |= 1 << 1;
    result[1] = color.value;
  }
  if (decoration != null) {
    result[0] |= 1 << 2;
    result[2] = decoration._mask;
  }
  if (decorationColor != null) {
    result[0] |= 1 << 3;
    result[3] = decorationColor.value;
  }
  if (decorationStyle != null) {
    result[0] |= 1 << 4;
    result[4] = decorationStyle.index;
  }
  if (decorationThickness != null) {
    result[0] |= 1 << 5;
  }
  if (fontWeight != null) {
    result[0] |= 1 << 6;
    result[5] = fontWeight.index;
  }
  if (fontStyle != null) {
    result[0] |= 1 << 7;
    result[6] = fontStyle.index;
  }
  if (textBaseline != null) {
    result[0] |= 1 << 8;
    result[7] = textBaseline.index;
  }
  if (fontFamily != null || (fontFamilyFallback != null && fontFamilyFallback.isNotEmpty)) {
    result[0] |= 1 << 9;
    // Passed separately to native.
  }
  if (fontSize != null) {
    result[0] |= 1 << 9;
    // Passed separately to native.
  }
  if (letterSpacing != null) {
    result[0] |= 1 << 10;
    // Passed separately to native.
  }
  if (wordSpacing != null) {
    result[0] |= 1 << 11;
    // Passed separately to native.
  }
  if (height != null) {
    result[0] |= 1 << 12;
    // Passed separately to native.
  }
  if (locale != null) {
    result[0] |= 1 << 13;
    // Passed separately to native.
  }
  if (background != null) {
    result[0] |= 1 << 14;
    // Passed separately to native.
  }
  if (foreground != null) {
    result[0] |= 1 << 15;
    // Passed separately to native.
  }
  if (shadows != null) {
    result[0] |= 1 << 16;
    // Passed separately to native.
  }
  return result;
}

/// Stub implementation. See docs in `../ui/`.
class TextStyle {
  /// Stub implementation. See docs in `../ui/`.
  TextStyle({
    Color color,
    TextDecoration decoration,
    Color decorationColor,
    TextDecorationStyle decorationStyle,
    double decorationThickness,
    FontWeight fontWeight,
    FontStyle fontStyle,
    TextBaseline textBaseline,
    String fontFamily,
    List<String> fontFamilyFallback,
    double fontSize,
    double letterSpacing,
    double wordSpacing,
    double height,
    Locale locale,
    Paint background,
    Paint foreground,
    List<Shadow> shadows,
  }) : assert(color == null || foreground == null,
         'Cannot provide both a color and a foreground\n'
         'The color argument is just a shorthand for "foreground: new Paint()..color = color".'
       ),
       _encoded = _encodeTextStyle(
         color,
         decoration,
         decorationColor,
         decorationStyle,
         decorationThickness,
         fontWeight,
         fontStyle,
         textBaseline,
         fontFamily,
         fontFamilyFallback,
         fontSize,
         letterSpacing,
         wordSpacing,
         height,
         locale,
         background,
         foreground,
         shadows,
       ),
       _fontFamily = fontFamily ?? '',
       _fontFamilyFallback = fontFamilyFallback,
       _fontSize = fontSize,
       _letterSpacing = letterSpacing,
       _wordSpacing = wordSpacing,
       _height = height,
       _decorationThickness = decorationThickness,
       _locale = locale,
       _background = background,
       _foreground = foreground,
       _shadows = shadows;

  final Int32List _encoded;
  final String _fontFamily;
  final List<String> _fontFamilyFallback;
  final double _fontSize;
  final double _letterSpacing;
  final double _wordSpacing;
  final double _height;
  final double _decorationThickness;
  final Locale _locale;
  final Paint _background;
  final Paint _foreground;
  final List<Shadow> _shadows;

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other is! TextStyle)
      return false;
    final TextStyle typedOther = other;
    if (_fontFamily != typedOther._fontFamily ||
        _fontSize != typedOther._fontSize ||
        _letterSpacing != typedOther._letterSpacing ||
        _wordSpacing != typedOther._wordSpacing ||
        _height != typedOther._height ||
        _decorationThickness != typedOther._decorationThickness ||
        _locale != typedOther._locale ||
        _background != typedOther._background ||
        _foreground != typedOther._foreground)
     return false;
    for (int index = 0; index < _encoded.length; index += 1) {
      if (_encoded[index] != typedOther._encoded[index])
        return false;
    }
    if (!_listEquals<Shadow>(_shadows, typedOther._shadows))
      return false;
    if (!_listEquals<String>(_fontFamilyFallback, typedOther._fontFamilyFallback))
      return false;
    return true;
  }

  @override
  int get hashCode => hashValues(hashList(_encoded), _fontFamily, _fontFamilyFallback, _fontSize, _letterSpacing, _wordSpacing, _height, _locale, _background, _foreground, _shadows, _decorationThickness);

  @override
  String toString() {
    return 'TextStyle('
             'color: ${              _encoded[0] & 0x00002 == 0x00002  ? new Color(_encoded[1])                  : "unspecified"}, '
             'decoration: ${         _encoded[0] & 0x00004 == 0x00004  ? new TextDecoration._(_encoded[2])       : "unspecified"}, '
             'decorationColor: ${    _encoded[0] & 0x00008 == 0x00008  ? new Color(_encoded[3])                  : "unspecified"}, '
             'decorationStyle: ${    _encoded[0] & 0x00010 == 0x00010  ? TextDecorationStyle.values[_encoded[4]] : "unspecified"}, '
             'decorationThickness: ${_encoded[0] & 0x00020 == 0x00020  ? _decorationThickness                    : "unspecified"}, '
             'fontWeight: ${         _encoded[0] & 0x00040 == 0x00040  ? FontWeight.values[_encoded[5]]          : "unspecified"}, '
             'fontStyle: ${          _encoded[0] & 0x00080 == 0x00080  ? FontStyle.values[_encoded[6]]           : "unspecified"}, '
             'textBaseline: ${       _encoded[0] & 0x00100 == 0x00100  ? TextBaseline.values[_encoded[7]]        : "unspecified"}, '
             'fontFamily: ${         _encoded[0] & 0x00200 == 0x00200
                                     && _fontFamily != null            ? _fontFamily                             : "unspecified"}, '
             'fontFamilyFallback: ${ _encoded[0] & 0x00200 == 0x00200
                                     && _fontFamilyFallback != null
                                     && _fontFamilyFallback.isNotEmpty ? _fontFamilyFallback                     : "unspecified"}, '
             'fontSize: ${           _encoded[0] & 0x00400 == 0x00400  ? _fontSize                               : "unspecified"}, '
             'letterSpacing: ${      _encoded[0] & 0x00800 == 0x00800  ? "${_letterSpacing}x"                    : "unspecified"}, '
             'wordSpacing: ${        _encoded[0] & 0x01000 == 0x01000  ? "${_wordSpacing}x"                      : "unspecified"}, '
             'height: ${             _encoded[0] & 0x02000 == 0x02000  ? "${_height}x"                           : "unspecified"}, '
             'locale: ${             _encoded[0] & 0x04000 == 0x04000  ? _locale                                 : "unspecified"}, '
             'background: ${         _encoded[0] & 0x08000 == 0x08000  ? _background                             : "unspecified"}, '
             'foreground: ${         _encoded[0] & 0x10000 == 0x10000  ? _foreground                             : "unspecified"}, '
             'shadows: ${            _encoded[0] & 0x20000 == 0x20000  ? _shadows                                : "unspecified"}'
           ')';
  }
}

// This encoding must match the C++ version ParagraphBuilder::build.
//
// The encoded array buffer has 6 elements.
//
//  - Element 0: A bit mask indicating which fields are non-null.
//    Bit 0 is unused. Bits 1-n are set if the corresponding index in the
//    encoded array is non-null.  The remaining bits represent fields that
//    are passed separately from the array.
//
//  - Element 1: The enum index of the |textAlign|.
//
//  - Element 2: The enum index of the |textDirection|.
//
//  - Element 3: The index of the |fontWeight|.
//
//  - Element 4: The enum index of the |fontStyle|.
//
//  - Element 5: The value of |maxLines|.
//
Int32List _encodeParagraphStyle(
  TextAlign textAlign,
  TextDirection textDirection,
  int maxLines,
  String fontFamily,
  double fontSize,
  double height,
  FontWeight fontWeight,
  FontStyle fontStyle,
  StrutStyle strutStyle,
  String ellipsis,
  Locale locale,
) {
  final Int32List result = new Int32List(6); // also update paragraph_builder.cc
  if (textAlign != null) {
    result[0] |= 1 << 1;
    result[1] = textAlign.index;
  }
  if (textDirection != null) {
    result[0] |= 1 << 2;
    result[2] = textDirection.index;
  }
  if (fontWeight != null) {
    result[0] |= 1 << 3;
    result[3] = fontWeight.index;
  }
  if (fontStyle != null) {
    result[0] |= 1 << 4;
    result[4] = fontStyle.index;
  }
  if (maxLines != null) {
    result[0] |= 1 << 5;
    result[5] = maxLines;
  }
  if (fontFamily != null) {
    result[0] |= 1 << 6;
    // Passed separately to native.
  }
  if (fontSize != null) {
    result[0] |= 1 << 7;
    // Passed separately to native.
  }
  if (height != null) {
    result[0] |= 1 << 8;
    // Passed separately to native.
  }
  if (strutStyle != null) {
    result[0] |= 1 << 9;
    // Passed separately to native.
  }
  if (ellipsis != null) {
    result[0] |= 1 << 10;
    // Passed separately to native.
  }
  if (locale != null) {
    result[0] |= 1 << 11;
    // Passed separately to native.
  }
  return result;
}

/// Stub implementation. See docs in `../ui/`.
class ParagraphStyle {
  /// Stub implementation. See docs in `../ui/`.
   //   See: https://github.com/flutter/flutter/issues/9819
  /// Stub implementation. See docs in `../ui/`.
  ParagraphStyle({
    TextAlign textAlign,
    TextDirection textDirection,
    int maxLines,
    String fontFamily,
    double fontSize,
    double height,
    FontWeight fontWeight,
    FontStyle fontStyle,
    StrutStyle strutStyle,
    String ellipsis,
    Locale locale,
  }) : _encoded = _encodeParagraphStyle(
         textAlign,
         textDirection,
         maxLines,
         fontFamily,
         fontSize,
         height,
         fontWeight,
         fontStyle,
         strutStyle,
         ellipsis,
         locale,
       ),
       _fontFamily = fontFamily,
       _fontSize = fontSize,
       _height = height,
       _strutStyle = strutStyle,
       _ellipsis = ellipsis,
       _locale = locale;

  final Int32List _encoded;
  final String _fontFamily;
  final double _fontSize;
  final double _height;
  final StrutStyle _strutStyle;
  final String _ellipsis;
  final Locale _locale;

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other.runtimeType != runtimeType)
      return false;
    final ParagraphStyle typedOther = other;
    if (_fontFamily != typedOther._fontFamily ||
        _fontSize != typedOther._fontSize ||
        _height != typedOther._height ||
        _strutStyle != typedOther._strutStyle ||
        _ellipsis != typedOther._ellipsis ||
        _locale != typedOther._locale)
     return false;
    for (int index = 0; index < _encoded.length; index += 1) {
      if (_encoded[index] != typedOther._encoded[index])
        return false;
    }
    return true;
  }

  @override
  int get hashCode => hashValues(hashList(_encoded), _fontFamily, _fontSize, _height, _ellipsis, _locale);

  @override
  String toString() {
    return '$runtimeType('
             'textAlign: ${     _encoded[0] & 0x002 == 0x002 ? TextAlign.values[_encoded[1]]     : "unspecified"}, '
             'textDirection: ${ _encoded[0] & 0x004 == 0x004 ? TextDirection.values[_encoded[2]] : "unspecified"}, '
             'fontWeight: ${    _encoded[0] & 0x008 == 0x008 ? FontWeight.values[_encoded[3]]    : "unspecified"}, '
             'fontStyle: ${     _encoded[0] & 0x010 == 0x010 ? FontStyle.values[_encoded[4]]     : "unspecified"}, '
             'maxLines: ${      _encoded[0] & 0x020 == 0x020 ? _encoded[5]                       : "unspecified"}, '
             'fontFamily: ${    _encoded[0] & 0x040 == 0x040 ? _fontFamily                       : "unspecified"}, '
             'fontSize: ${      _encoded[0] & 0x080 == 0x080 ? _fontSize                         : "unspecified"}, '
             'height: ${        _encoded[0] & 0x100 == 0x100 ? "${_height}x"                     : "unspecified"}, '
             'ellipsis: ${      _encoded[0] & 0x200 == 0x200 ? "\"$_ellipsis\""                  : "unspecified"}, '
             'locale: ${        _encoded[0] & 0x400 == 0x400 ? _locale                           : "unspecified"}'
           ')';
  }
}

// Serialize strut properties into ByteData. This encoding errs towards
// compactness. The first 8 bits is a bitmask that records which properties
// are null. The rest of the values are encoded in the same order encountered
// in the bitmask. The final returned value truncates any unused bytes
// at the end.
//
// We serialize this more thoroughly than ParagraphStyle because it is
// much more likely that the strut is empty/null and we wish to add
// minimal overhead for non-strut cases.
ByteData _encodeStrut(
  String fontFamily,
  List<String> fontFamilyFallback,
  double fontSize,
  double height,
  double leading,
  FontWeight fontWeight,
  FontStyle fontStyle,
  bool forceStrutHeight) {
  if (fontFamily == null &&
    fontSize == null &&
    height == null &&
    leading == null &&
    fontWeight == null &&
    fontStyle == null &&
    forceStrutHeight == null)
    return ByteData(0);

  final ByteData data = ByteData(15); // Max size is 15 bytes
  int bitmask = 0; // 8 bit mask
  int byteCount = 1;
  if (fontWeight != null) {
    bitmask |= 1 << 0;
    data.setInt8(byteCount, fontWeight.index);
    byteCount += 1;
  }
  if (fontStyle != null) {
    bitmask |= 1 << 1;
    data.setInt8(byteCount, fontStyle.index);
    byteCount += 1;
  }
  if (fontFamily != null || (fontFamilyFallback != null && fontFamilyFallback.isNotEmpty)){
    bitmask |= 1 << 2;
    // passed separately to native
  }
  if (fontSize != null) {
    bitmask |= 1 << 3;
    data.setFloat32(byteCount, fontSize, _kFakeHostEndian);
    byteCount += 4;
  }
  if (height != null) {
    bitmask |= 1 << 4;
    data.setFloat32(byteCount, height, _kFakeHostEndian);
    byteCount += 4;
  }
  if (leading != null) {
    bitmask |= 1 << 5;
    data.setFloat32(byteCount, leading, _kFakeHostEndian);
    byteCount += 4;
  }
  if (forceStrutHeight != null) {
    bitmask |= 1 << 6;
    // We store this boolean directly in the bitmask since there is
    // extra space in the 16 bit int.
    bitmask |= (forceStrutHeight ? 1 : 0) << 7;
  }

  data.setInt8(0, bitmask);

  return ByteData.view(data.buffer, 0,  byteCount);
}

class StrutStyle {
  /// Stub implementation. See docs in `../ui/`.
  StrutStyle({
    String fontFamily,
    List<String> fontFamilyFallback,
    double fontSize,
    double height,
    double leading,
    FontWeight fontWeight,
    FontStyle fontStyle,
    bool forceStrutHeight,
  }) : _encoded = _encodeStrut(
         fontFamily,
         fontFamilyFallback,
         fontSize,
         height,
         leading,
         fontWeight,
         fontStyle,
         forceStrutHeight,
       ),
       _fontFamily = fontFamily,
       _fontFamilyFallback = fontFamilyFallback;

  final ByteData _encoded; // Most of the data for strut is encoded.
  final String _fontFamily;
  final List<String> _fontFamilyFallback;


  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other.runtimeType != runtimeType)
      return false;
    final StrutStyle typedOther = other;
    if (_fontFamily != typedOther._fontFamily)
     return false;
    final Int8List encodedList = _encoded.buffer.asInt8List();
    final Int8List otherEncodedList = typedOther._encoded.buffer.asInt8List();
    for (int index = 0; index < _encoded.lengthInBytes; index += 1) {
      if (encodedList[index] != otherEncodedList[index])
        return false;
    }
    if (!_listEquals<String>(_fontFamilyFallback, typedOther._fontFamilyFallback))
      return false;
    return true;
  }

  @override
  int get hashCode => hashValues(hashList(_encoded.buffer.asInt8List()), _fontFamily);

}

/// Stub implementation. See docs in `../ui/`.
// The order of this enum must match the order of the values in TextDirection.h's TextDirection.
enum TextDirection {
  /// Stub implementation. See docs in `../ui/`.
  rtl,

  /// Stub implementation. See docs in `../ui/`.
  ltr,
}

/// Stub implementation. See docs in `../ui/`.
class TextBox {
  /// Stub implementation. See docs in `../ui/`.
  const TextBox.fromLTRBD(
    this.left,
    this.top,
    this.right,
    this.bottom,
    this.direction,
  );

  TextBox._(
    this.left,
    this.top,
    this.right,
    this.bottom,
    int directionIndex,
  ) : direction = TextDirection.values[directionIndex];

  /// Stub implementation. See docs in `../ui/`.
  final double left;

  /// Stub implementation. See docs in `../ui/`.
  final double top;

  /// Stub implementation. See docs in `../ui/`.
  final double right;

  /// Stub implementation. See docs in `../ui/`.
  final double bottom;

  /// Stub implementation. See docs in `../ui/`.
  final TextDirection direction;

  /// Stub implementation. See docs in `../ui/`.
  Rect toRect() => new Rect.fromLTRB(left, top, right, bottom);

  /// Stub implementation. See docs in `../ui/`.
  double get start {
    return (direction == TextDirection.ltr) ? left : right;
  }

  /// Stub implementation. See docs in `../ui/`.
  double get end {
    return (direction == TextDirection.ltr) ? right : left;
  }

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other))
      return true;
    if (other.runtimeType != runtimeType)
      return false;
    final TextBox typedOther = other;
    return typedOther.left == left
        && typedOther.top == top
        && typedOther.right == right
        && typedOther.bottom == bottom
        && typedOther.direction == direction;
  }

  @override
  int get hashCode => hashValues(left, top, right, bottom, direction);

  @override
  String toString() => 'TextBox.fromLTRBD(${left.toStringAsFixed(1)}, ${top.toStringAsFixed(1)}, ${right.toStringAsFixed(1)}, ${bottom.toStringAsFixed(1)}, $direction)';
}

/// Stub implementation. See docs in `../ui/`.
enum TextAffinity {
  /// Stub implementation. See docs in `../ui/`.
  upstream,

  /// Stub implementation. See docs in `../ui/`.
  downstream,
}

/// Stub implementation. See docs in `../ui/`.
class TextPosition {
  /// Stub implementation. See docs in `../ui/`.
  const TextPosition({
    this.offset,
    this.affinity: TextAffinity.downstream,
  }) : assert(offset != null),
       assert(affinity != null);

  /// Stub implementation. See docs in `../ui/`.
  final int offset;

  /// Stub implementation. See docs in `../ui/`.
  final TextAffinity affinity;

  @override
  bool operator ==(dynamic other) {
    if (other.runtimeType != runtimeType)
      return false;
    final TextPosition typedOther = other;
    return typedOther.offset == offset
        && typedOther.affinity == affinity;
  }

  @override
  int get hashCode => hashValues(offset, affinity);

  @override
  String toString() {
    return '$runtimeType(offset: $offset, affinity: $affinity)';
  }
}

/// Stub implementation. See docs in `../ui/`.
class ParagraphConstraints {
  /// Stub implementation. See docs in `../ui/`.
  const ParagraphConstraints({
    this.width,
  }) : assert(width != null);

  /// Stub implementation. See docs in `../ui/`.
  final double width;

  @override
  bool operator ==(dynamic other) {
    if (other.runtimeType != runtimeType)
      return false;
    final ParagraphConstraints typedOther = other;
    return typedOther.width == width;
  }

  @override
  int get hashCode => width.hashCode;

  @override
  String toString() => '$runtimeType(width: $width)';
}

/// Stub implementation. See docs in `../ui/`.
enum BoxHeightStyle {
    /// Stub implementation. See docs in `../ui/`.
    tight,

    /// Stub implementation. See docs in `../ui/`.
    max,

    /// Stub implementation. See docs in `../ui/`.
    includeLineSpacingMiddle,

    /// Stub implementation. See docs in `../ui/`.
    includeLineSpacingTop,

    /// Stub implementation. See docs in `../ui/`.
    includeLineSpacingBottom,
}

/// Stub implementation. See docs in `../ui/`.
enum BoxWidthStyle {
    // Provide tight bounding boxes that fit widths to the runs of each line
    // independently.
    tight,

    /// Stub implementation. See docs in `../ui/`.
    max,
}

/// Stub implementation. See docs in `../ui/`.
class Paragraph {
  /// Stub implementation. See docs in `../ui/`.
  Paragraph._();

  /// Stub implementation. See docs in `../ui/`.
  double get width {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get height {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get minIntrinsicWidth {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get maxIntrinsicWidth {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get alphabeticBaseline {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get ideographicBaseline {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  bool get didExceedMaxLines {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void layout(ParagraphConstraints constraints) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  List<TextBox> getBoxesForRange(int start, int end, {BoxHeightStyle boxHeightStyle = BoxHeightStyle.tight, BoxWidthStyle boxWidthStyle = BoxWidthStyle.tight}) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  TextPosition getPositionForOffset(Offset offset) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  List<int> getWordBoundary(int offset) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  double get longestLine {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
class ParagraphBuilder {

  /// Stub implementation. See docs in `../ui/`.
  ParagraphBuilder(ParagraphStyle style);

  /// Stub implementation. See docs in `../ui/`.
  void pushStyle(TextStyle style) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void pop() {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  void addText(String text) {
    throw UnimplementedError();
  }

  /// Stub implementation. See docs in `../ui/`.
  Paragraph build() {
    throw UnimplementedError();
  }
}

/// Stub implementation. See docs in `../ui/`.
Future<void> loadFontFromList(Uint8List list, {String fontFamily}) {
  throw UnimplementedError();
}
