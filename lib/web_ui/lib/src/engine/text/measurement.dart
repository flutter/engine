// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import '../../engine.dart' show registerHotRestartListener;
import '../dom.dart';
import '../embedder.dart';
import 'canvas_paragraph.dart';
import 'line_breaker.dart';
import 'paragraph.dart';
import 'ruler.dart';

// TODO(yjbanov): this is a hack we use to compute ideographic baseline; this
//                number is the ratio ideographic/alphabetic for font Ahem,
//                which matches the Flutter number. It may be completely wrong
//                for any other font. We'll need to eventually fix this. That
//                said Flutter doesn't seem to use ideographic baseline for
//                anything as of this writing.
const double baselineRatioHack = 1.1662499904632568;

/// Hosts ruler DOM elements in a hidden container under a `root` [html.Node].
///
/// The `root` [html.Node] is optional. Defaults to [flutterViewEmbedder.glassPaneShadow].
class RulerHost {
  RulerHost({html.Node? root}) {
    _rulerHost.style
      ..position = 'fixed'
      ..visibility = 'hidden'
      ..overflow = 'hidden'
      ..top = '0'
      ..left = '0'
      ..width = '0'
      ..height = '0';

    (root ?? flutterViewEmbedder.glassPaneShadow!.node).append(_rulerHost);
    registerHotRestartListener(dispose);
  }

  /// Hosts a cache of rulers that measure text.
  ///
  /// This element exists purely for organizational purposes. Otherwise the
  /// rulers would be attached to the `<body>` element polluting the element
  /// tree and making it hard to navigate. It does not serve any functional
  /// purpose.
  final html.Element _rulerHost = html.Element.tag('flt-ruler-host');

  /// Releases the resources used by this [RulerHost].
  ///
  /// After this is called, this object is no longer usable.
  void dispose() {
    _rulerHost.remove();
  }

  /// Adds an element used for measuring text as a child of [_rulerHost].
  void addElement(html.HtmlElement element) {
    _rulerHost.append(element);
  }
}

// These global variables are used to memoize calls to [measureSubstring]. They
// are used to remember the last arguments passed to it, and the last return
// value.
// They are being initialized so that the compiler knows they'll never be null.
int _lastStart = -1;
int _lastEnd = -1;
String _lastText = '';
String _lastCssFont = '';
double _lastWidth = -1;

/// Measures the width of the substring of [text] starting from the index
/// [start] (inclusive) to [end] (exclusive).
///
/// This method assumes that the correct font has already been set on
/// [_canvasContext].
double measureSubstring(
  DomCanvasRenderingContext2D _canvasContext,
  String text,
  int start,
  int end, {
  double? letterSpacing,
}) {
  assert(0 <= start);
  assert(start <= end);
  assert(end <= text.length);

  if (start == end) {
    return 0;
  }

  final String cssFont = _canvasContext.font;
  double width;

  // TODO(mdebbar): Explore caching all widths in a map, not only the last one.
  if (start == _lastStart &&
      end == _lastEnd &&
      text == _lastText &&
      cssFont == _lastCssFont) {
    // Reuse the previously calculated width if all factors that affect width
    // are unchanged. The only exception is letter-spacing. We always add
    // letter-spacing to the width later below.
    width = _lastWidth;
  } else {
    final String sub =
      start == 0 && end == text.length ? text : text.substring(start, end);
    width = _canvasContext.measureText(sub).width!.toDouble();
  }

  _lastStart = start;
  _lastEnd = end;
  _lastText = text;
  _lastCssFont = cssFont;
  _lastWidth = width;

  // Now add letter spacing to the width.
  letterSpacing ??= 0.0;
  if (letterSpacing != 0.0) {
    width += letterSpacing * (end - start);
  }

  // What we are doing here is we are rounding to the nearest 2nd decimal
  // point. So 39.999423 becomes 40, and 11.243982 becomes 11.24.
  // The reason we are doing this is because we noticed that canvas API has a
  // ±0.001 error margin.
  return _roundWidth(width);
}

double _roundWidth(double width) {
  // What we are doing here is we are rounding to the nearest 2nd decimal
  // point. So 39.999423 becomes 40, and 11.243982 becomes 11.24.
  // The reason we are doing this is because we noticed that canvas API has a
  // ±0.001 error margin.
  return (width * 100).round() / 100;
}

/// Immutable data class that holds measurement results for a piece of text.
class TextMetrics {
  const TextMetrics._({
    required this.ascent,
    required this.descent,
    required this.textAscent,
    required this.textDescent,
    required this.widthExcludingTrailingSpaces,
    required this.widthIncludingTrailingSpaces,
  });

  factory TextMetrics._fromCanvasTextMetrics({
    required DomTextMetrics excludingTrailingSpaces,
    required DomTextMetrics includingTrailingSpaces,
    required EngineTextStyle style,
  }) {
    final num height = style.height != null
        ? (style.height! * style.fontSize!)
        : (excludingTrailingSpaces.fontBoundingBoxAscent! +
            excludingTrailingSpaces.fontBoundingBoxDescent!);

    print('fullHeight: $height');

    print('ratio (line-height?): ${height / style.fontSize!}');

    final double textAscent = excludingTrailingSpaces.actualBoundingBoxAscent!.toDouble();
    final double textDescent = excludingTrailingSpaces.actualBoundingBoxDescent!.toDouble();

    // Divide the extra height equally between ascent and descent.
    final double extraHeight = height - (textAscent + textDescent);
    final double ascent = textAscent + extraHeight / 2;
    final double descent = height - ascent;

    return TextMetrics._(
      ascent: ascent,
      descent: descent,
      textAscent: textAscent,
      textDescent: textDescent,
      widthExcludingTrailingSpaces:
          _roundWidth(excludingTrailingSpaces.width!.toDouble()),
      widthIncludingTrailingSpaces:
          _roundWidth(includingTrailingSpaces.width!.toDouble()),
    );
  }

  factory TextMetrics.forPlaceholder({
    required PlaceholderSpan placeholder,
    required double ascent,
    required double descent,
  }) {
    assert(placeholder.height == ascent + descent);
    return TextMetrics._(
      ascent: ascent,
      descent: descent,
      textAscent: ascent,
      textDescent: descent,
      widthExcludingTrailingSpaces: placeholder.width,
      widthIncludingTrailingSpaces: placeholder.width,
    );
  }

  static const TextMetrics zero = TextMetrics._(
    ascent: 0,
    descent: 0,
    textAscent: 0,
    textDescent: 0,
    widthExcludingTrailingSpaces: 0,
    widthIncludingTrailingSpaces: 0,
  );

  /// The rise from the baseline as calculated from the font and style for this text.
  final double ascent;

  /// The drop from the baseline as calculated from the font and style for this text.
  final double descent;

  /// The rise from the baseline to the top of the bounding box of the measured text.
  final double textAscent;

  /// The drop from the baseline to the bottom of the bounding box of the measured text.
  final double textDescent;

  /// The width of the measured text, not including trailing spaces.
  final double widthExcludingTrailingSpaces;

  /// The width of the measured text, including any trailing spaces.
  final double widthIncludingTrailingSpaces;

  /// The width of trailing spaces in the measured text.
  double get widthOfTrailingSpaces => widthIncludingTrailingSpaces - widthExcludingTrailingSpaces;

  /// The total height as calculated from the font and style for this text.
  double get height => ascent + descent;

  /// The height of the measured text's bounding box.
  double get textHeight => textAscent + textDescent;
}

/// Helper class that measures text in isolation and provides caching for better
/// performance.
class CachedMeasurements {
  CachedMeasurements(this._canvasContext);

  final DomCanvasRenderingContext2D _canvasContext;

  /// A cache that maps: style -> text -> TextMetrics
  final Map<String, Map<String, TextMetrics>> _cache =
      <String, Map<String, TextMetrics>>{};

  /// Measures the range of text between two line break results.
  TextMetrics measureRange(
    String text,
    EngineTextStyle style,
    LineBreakResult start,
    LineBreakResult end,
  ) {
    return rawMeasure(
      text: text,
      style: style,
      start: start.index,
      endExcludingTrailingSpaces: end.indexWithoutTrailingSpaces,
      // Trailing new lines should never be included in measurements.
      endIncludingTrailingSpaces: end.indexWithoutTrailingNewlines,
    );
  }

  TextMetrics measureText(String text, EngineTextStyle style) {
    return rawMeasure(
      text: text,
      style: style,
      start: 0,
      endExcludingTrailingSpaces: text.length,
      endIncludingTrailingSpaces: text.length,
    );
  }

  TextMetrics measureSubstring(
    String text,
    EngineTextStyle style,
    int start,
    int end,
  ) {
    return rawMeasure(
      text: text,
      style: style,
      start: start,
      endExcludingTrailingSpaces: end,
      endIncludingTrailingSpaces: end,
    );
  }

  TextMetrics rawMeasure({
    required String text,
    required EngineTextStyle style,
    required int start,
    required int endExcludingTrailingSpaces,
    required int endIncludingTrailingSpaces,
  }) {
    // 0 <= start <= endExcludingTrailingSpace <= endIncludingTrailingSpaces <= text.length.
    assert(0 <= start);
    assert(start <= endExcludingTrailingSpaces);
    assert(endExcludingTrailingSpaces <= endIncludingTrailingSpaces);
    assert(endIncludingTrailingSpaces <= text.length);

    // Empty range.
    if (start == endIncludingTrailingSpaces) {
      return TextMetrics.zero;
    }

    final String cssFontString = style.cssFontString;
    print('font: "$cssFontString"');

    final String textIncludingTrailingSpaces =
        _extractSubstring(text, start, endIncludingTrailingSpaces);

    final Map<String, TextMetrics> fontCache =
        _cache.putIfAbsent(cssFontString, () => <String, TextMetrics>{});
    final TextMetrics? cachedMetrics = fontCache[textIncludingTrailingSpaces];
    if (cachedMetrics != null) {
      return cachedMetrics;
    }

    if (_canvasContext.font != cssFontString) {
      _canvasContext.font = cssFontString;
    }

    // TODO(mdebbar): LETTER SPACING <-> CACHE!!

    // // Now add letter spacing to the width.
    // letterSpacing ??= 0.0;
    // if (letterSpacing != 0.0) {
    //   width += letterSpacing * (end - start);
    // }

    // TODO(mdebbar): FALLBACK TO DOM IF BROWSER DOESN'T SUPPORT HEIGHT METRICS.

    final bool hasTrailingSpaces =
        endIncludingTrailingSpaces != endExcludingTrailingSpaces;

    final DomTextMetrics metricsIncludingTrailingSpaces =
        _canvasContext.measureText(textIncludingTrailingSpaces);
    final DomTextMetrics metricsExcludingTrailingSpaces;

    if (hasTrailingSpaces) {
      final String textExcludingTrailingSpaces =
          _extractSubstring(text, start, endExcludingTrailingSpaces);
      metricsExcludingTrailingSpaces =
          _canvasContext.measureText(textExcludingTrailingSpaces);
    } else {
      metricsExcludingTrailingSpaces = metricsIncludingTrailingSpaces;
    }

    // print('text: "$textIncludingTrailingSpaces"');
    // print(
    //     'actual: (${_roundWidth(metricsIncludingTrailingSpaces.actualBoundingBoxAscent!.toDouble())}, ${_roundWidth(metricsIncludingTrailingSpaces.actualBoundingBoxDescent!.toDouble())})');
    // print(
    //     'font: (${_roundWidth(metricsIncludingTrailingSpaces.fontBoundingBoxAscent!.toDouble())}, ${_roundWidth(metricsIncludingTrailingSpaces.fontBoundingBoxDescent!.toDouble())})');
    // print(
    //     'ratio: ${(metricsIncludingTrailingSpaces.fontBoundingBoxAscent! + metricsIncludingTrailingSpaces.fontBoundingBoxDescent!) / (metricsIncludingTrailingSpaces.actualBoundingBoxAscent! + metricsIncludingTrailingSpaces.actualBoundingBoxDescent!)}');
    // print(
    //     'font ratio: ${(metricsIncludingTrailingSpaces.fontBoundingBoxAscent! + metricsIncludingTrailingSpaces.fontBoundingBoxDescent!) / style.fontSize!}');
    final TextMetrics metrics = TextMetrics._fromCanvasTextMetrics(
      excludingTrailingSpaces: metricsExcludingTrailingSpaces,
      includingTrailingSpaces: metricsIncludingTrailingSpaces,
      style: style,
    );

    fontCache[textIncludingTrailingSpaces] = metrics;

    return metrics;
  }
}

String _extractSubstring(String text, int start, int end) {
  // Optimization to avoid calling `substring` if the range covers the entire
  // string.
  return start == 0 && end == text.length ? text : text.substring(start, end);
}
