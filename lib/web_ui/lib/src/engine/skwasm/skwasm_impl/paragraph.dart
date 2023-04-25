// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_unused_constructor_parameters

import 'dart:ffi';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
import 'package:ui/src/engine/skwasm/skwasm_impl/raw/text/raw_text_style.dart';
import 'package:ui/ui.dart' as ui;

// TODO(jacksongardner): implement everything in this file
class SkwasmLineMetrics implements ui.LineMetrics {
  factory SkwasmLineMetrics({
    required bool hardBreak,
    required double ascent,
    required double descent,
    required double unscaledAscent,
    required double height,
    required double width,
    required double left,
    required double baseline,
    required int lineNumber,
  }) {
    throw UnimplementedError();
  }

  @override
  bool get hardBreak {
    throw UnimplementedError();
  }

  @override
  double get ascent {
    throw UnimplementedError();
  }

  @override
  double get descent {
    throw UnimplementedError();
  }

  @override
  double get unscaledAscent {
    throw UnimplementedError();
  }

  @override
  double get height {
    throw UnimplementedError();
  }

  @override
  double get width {
    throw UnimplementedError();
  }

  @override
  double get left {
    throw UnimplementedError();
  }

  @override
  double get baseline {
    throw UnimplementedError();
  }

  @override
  int get lineNumber {
    throw UnimplementedError();
  }
}

class SkwasmParagraph implements ui.Paragraph {
  @override
  double get width {
    return 0.0;
  }

  @override
  double get height {
    return 0.0;
  }

  @override
  double get longestLine {
    return 0.0;
  }

  @override
  double get minIntrinsicWidth {
    return 0.0;
  }

  @override
  double get maxIntrinsicWidth {
    return 0.0;
  }

  @override
  double get alphabeticBaseline {
    return 0.0;
  }

  @override
  double get ideographicBaseline {
    return 0.0;
  }

  @override
  bool get didExceedMaxLines {
    return false;
  }

  @override
  void layout(ui.ParagraphConstraints constraints) {
  }

  @override
  List<ui.TextBox> getBoxesForRange(int start, int end,
      {ui.BoxHeightStyle boxHeightStyle = ui.BoxHeightStyle.tight,
      ui.BoxWidthStyle boxWidthStyle = ui.BoxWidthStyle.tight}) {
    return <ui.TextBox>[];
  }

  @override
  ui.TextPosition getPositionForOffset(ui.Offset offset) {
    return const ui.TextPosition(offset: 0);
  }

  @override
  ui.TextRange getWordBoundary(ui.TextPosition position) {
    return const ui.TextRange(start: 0, end: 0);
  }

  @override
  ui.TextRange getLineBoundary(ui.TextPosition position) {
    return const ui.TextRange(start: 0, end: 0);
  }

  @override
  List<ui.TextBox> getBoxesForPlaceholders() {
    return <ui.TextBox>[];
  }

  @override
  List<SkwasmLineMetrics> computeLineMetrics() {
    return <SkwasmLineMetrics>[];
  }

  @override
  bool get debugDisposed => false;

  @override
  void dispose() {
  }
}

class SkwasmParagraphStyle implements ui.ParagraphStyle {
}

class SkwasmTextStyle implements ui.TextStyle {
  factory SkwasmTextStyle({
    ui.Color? color,
    ui.TextDecoration? decoration,
    ui.Color? decorationColor,
    ui.TextDecorationStyle? decorationStyle,
    double? decorationThickness,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    ui.TextBaseline? textBaseline,
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? letterSpacing,
    double? wordSpacing,
    double? height,
    ui.TextLeadingDistribution? leadingDistribution,
    ui.Locale? locale,
    ui.Paint? background,
    ui.Paint? foreground,
    List<ui.Shadow>? shadows,
    List<ui.FontFeature>? fontFeatures,
    List<ui.FontVariation>? fontVariations,
  }) {
    final TextStyleHandle handle = textStyleCreate();
    if (color != null) {
      textStyleSetColor(handle, color.value);
    }
    if (decoration != null) {
      textStyleSetDecoration(handle, decoration.maskValue);
    }
    if (decorationColor != null) {
      textStyleSetDecorationColor(handle, decorationColor.value);
    }
    if (decorationStyle != null) {
      textStyleSetDecorationStyle(handle, decorationStyle.index);
    }
    if (decorationThickness != null) {
      textStyleSetDecorationThickness(handle, decorationThickness);
    }
    if (fontWeight != null || fontStyle != null) {
      fontWeight ??= ui.FontWeight.normal;
      fontStyle ??= ui.FontStyle.normal;
      textStyleSetFontStyle(handle, fontWeight.value, fontStyle.index);
    }
    if (textBaseline != null) {
      textStyleSetTextBaseline(handle, textBaseline.index);
    }
    if (fontFamily != null || 
      (fontFamilyFallback != null  && fontFamilyFallback.isNotEmpty)) {
      int count = fontFamily != null ? 1 : 0;
      if (fontFamilyFallback != null) {
        count += fontFamilyFallback.length;
      }
      withStackScope((StackScope scope) {
        final Pointer<SkStringHandle> familiesPtr = 
          scope.allocPointerArray(count).cast<SkStringHandle>();
        int nativeIndex = 0;
        if (fontFamily != null) {
          familiesPtr[nativeIndex] = skStringFromDartString(fontFamily);
          nativeIndex++;
        }
        if (fontFamilyFallback != null) {
          for (final String family in fontFamilyFallback) {
            familiesPtr[nativeIndex] = skStringFromDartString(family);   
            nativeIndex++;
          }
        }
        textStyleSetFontFamilies(handle, familiesPtr, count);
        for (int i = 0; i < count; i++) {
          skStringFree(familiesPtr[i]);
        }
      });
    }
    if (fontSize != null) {
      textStyleSetFontSize(handle, fontSize);
    }
    if (letterSpacing != null) {
      textStyleSetLetterSpacing(handle, letterSpacing);
    }
    if (wordSpacing != null) {
      textStyleSetWordSpacing(handle, wordSpacing);
    }
    if (height != null) {
      textStyleSetHeight(handle, height);
    }
    if (leadingDistribution != null) {
      textStyleSetHalfLeading(
        handle,
        leadingDistribution == ui.TextLeadingDistribution.even
      );
    }
    if (locale != null) {
      final SkStringHandle localeHandle = 
        skStringFromDartString(locale.toLanguageTag());
      textStyleSetLocale(handle, localeHandle);
      skStringFree(localeHandle);
    }
    if (background != null) {
      background as SkwasmPaint;
      textStyleSetBackground(handle, background.handle);
    }
    if (foreground != null) {
      foreground as SkwasmPaint;
      textStyleSetForeground(handle, foreground.handle);
    }
    if (shadows != null) {
      for (final ui.Shadow shadow in shadows) {
        textStyleAddShadow(
          handle,
          shadow.color.value,
          shadow.offset.dx,
          shadow.offset.dy,
          shadow.blurSigma,
        );
      }
    }
    if (fontFeatures != null) {
      for (final ui.FontFeature feature in fontFeatures) {
        final SkStringHandle featureName = skStringFromDartString(feature.feature);
        textStyleAddFontFeature(handle, featureName, feature.value);
        skStringFree(featureName);
      }
    }
    // TODO(jacksongardner): Set font variations
    return SkwasmTextStyle._(handle);
  }

  SkwasmTextStyle._(this.handle);

  final TextStyleHandle handle;
}

class SkwasmParagraphBuilder implements ui.ParagraphBuilder {
  @override
  void addPlaceholder(
    double width,
    double height,
    ui.PlaceholderAlignment alignment, {
    double scale = 1.0,
    double? baselineOffset,
    ui.TextBaseline? baseline
  }) {
  }

  @override
  void addText(String text) {
  }

  @override
  ui.Paragraph build() {
    return SkwasmParagraph();
  }

  @override
  int get placeholderCount => 0;

  @override
  List<double> get placeholderScales => <double>[];

  @override
  void pop() {
  }

  @override
  void pushStyle(ui.TextStyle style) {
  }
}
