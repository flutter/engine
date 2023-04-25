// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_unused_constructor_parameters

import 'dart:ffi';

import 'package:ui/src/engine/skwasm/skwasm_impl.dart';
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

void withScopedFontList(
    List<String> fontFamilies,
  void Function(Pointer<SkStringHandle>, int) callback) {
  withStackScope((StackScope scope) {
    final Pointer<SkStringHandle> familiesPtr =
      scope.allocPointerArray(fontFamilies.length).cast<SkStringHandle>();
    int nativeIndex = 0;
    for (int i = 0; i < fontFamilies.length; i++) {
      familiesPtr[nativeIndex] = skStringFromDartString(fontFamilies[i]);
      nativeIndex++;
    }
    callback(familiesPtr, fontFamilies.length);
    for (int i = 0; i < fontFamilies.length; i++) {
      skStringFree(familiesPtr[i]);
    }
  });
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
    if (fontFamily != null || fontFamilyFallback != null) {
      final List<String> fontFamilies = <String>[
        if (fontFamily != null) fontFamily,
        if (fontFamilyFallback != null) ...fontFamilyFallback,
      ];
      if (fontFamilies.isNotEmpty) {
        withScopedFontList(fontFamilies,
          (Pointer<SkStringHandle> families, int count) =>
            textStyleSetFontFamilies(handle, families, count));
      }
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

class SkwasmStrutStyle implements ui.StrutStyle {
  factory SkwasmStrutStyle({
    String? fontFamily,
    List<String>? fontFamilyFallback,
    double? fontSize,
    double? height,
    ui.TextLeadingDistribution? leadingDistribution,
    double? leading,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    bool? forceStrutHeight,
  }) {
    final StrutStyleHandle handle = strutStyleCreate();
    if (fontFamily != null || fontFamilyFallback != null) {
      final List<String> fontFamilies = <String>[
        if (fontFamily != null) fontFamily,
        if (fontFamilyFallback != null) ...fontFamilyFallback,
      ];
      if (fontFamilies.isNotEmpty) {
        withScopedFontList(fontFamilies,
          (Pointer<SkStringHandle> families, int count) =>
            strutStyleSetFontFamilies(handle, families, count));
      }
    }
    if (fontSize != null) {
      strutStyleSetFontSize(handle, fontSize);
    }
    if (height != null) {
      strutStyleSetHeight(handle, height);
    }
    if (leadingDistribution != null) {
      strutStyleSetHalfLeading(
        handle,
        leadingDistribution == ui.TextLeadingDistribution.even);
    }
    if (leading != null) {
      strutStyleSetLeading(handle, leading);
    }
    if (fontWeight != null || fontStyle != null) {
      fontWeight ??= ui.FontWeight.normal;
      fontStyle ??= ui.FontStyle.normal;
      strutStyleSetFontStyle(handle, fontWeight.value, fontStyle.index);
    }
    if (forceStrutHeight != null) {
      strutStyleSetForceStrutHeight(handle, forceStrutHeight);
    }
    return SkwasmStrutStyle._(handle);
  }

  SkwasmStrutStyle._(this.handle);

  final StrutStyleHandle handle;
}

class SkwasmParagraphStyle implements ui.ParagraphStyle {
  factory SkwasmParagraphStyle({
    ui.TextAlign? textAlign,
    ui.TextDirection? textDirection,
    int? maxLines,
    String? fontFamily,
    double? fontSize,
    double? height,
    ui.TextHeightBehavior? textHeightBehavior,
    ui.FontWeight? fontWeight,
    ui.FontStyle? fontStyle,
    ui.StrutStyle? strutStyle,
    String? ellipsis,
    ui.Locale? locale,
  }) {
    final ParagraphStyleHandle handle = paragraphStyleCreate();
    if (textAlign != null) {
      paragraphStyleSetTextAlign(handle, textAlign.index);
    }
    if (textDirection != null) {
      paragraphStyleSetTextDirection(handle, textDirection.index);
    }
    if (maxLines != null) {
      paragraphStyleSetMaxLines(handle, maxLines);
    }
    if (height != null) {
      paragraphStyleSetHeight(handle, height);
    }
    if (textHeightBehavior != null) {
      paragraphStyleSetTextHeightBehavior(
        handle,
        textHeightBehavior.applyHeightToFirstAscent,
        textHeightBehavior.applyHeightToLastDescent,
      );
    }
    if (ellipsis != null) {
      final SkStringHandle ellipsisHandle = skStringFromDartString(ellipsis);
      paragraphStyleSetEllipsis(handle, ellipsisHandle);
      skStringFree(ellipsisHandle);
    }
    if (strutStyle != null) {
      strutStyle as SkwasmStrutStyle;
      paragraphStyleSetStrutStyle(handle, strutStyle.handle);
    }
    if (fontFamily != null ||
      fontSize != null ||
      fontWeight != null ||
      fontStyle != null ||
      textHeightBehavior != null ||
      locale != null) {
      final TextStyleHandle textStyleHandle = textStyleCreate();
      if (fontFamily != null) {
        withScopedFontList(<String>[fontFamily], 
          (Pointer<SkStringHandle> families, int count) =>
            textStyleSetFontFamilies(textStyleHandle, families, count));
      }
      if (fontSize != null) {
        textStyleSetFontSize(textStyleHandle, fontSize);
      }
      if (fontWeight != null || fontStyle != null) {
        fontWeight ??= ui.FontWeight.normal;
        fontStyle ??= ui.FontStyle.normal;
        textStyleSetFontStyle(textStyleHandle, fontWeight.value, fontStyle.index);
      }
      if (textHeightBehavior != null) {
        textStyleSetHalfLeading(
          textStyleHandle,
          textHeightBehavior.leadingDistribution == ui.TextLeadingDistribution.even,
        );
      }
      if (locale != null) {
        final SkStringHandle localeHandle =
        skStringFromDartString(locale.toLanguageTag());
        textStyleSetLocale(textStyleHandle, localeHandle);
        skStringFree(localeHandle);
      }
      paragraphStyleSetTextStyle(handle, textStyleHandle);
      textStyleDispose(textStyleHandle);
    }
    return SkwasmParagraphStyle._(handle);
  }

  SkwasmParagraphStyle._(this.handle);

  final ParagraphStyleHandle handle;
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
