// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../html/bitmap_canvas.dart';
import '../html/painting.dart';
import 'canvas_paragraph.dart';
import 'layout_fragmenter.dart';
import 'paragraph.dart';

/// Responsible for painting a [CanvasParagraph] on a [BitmapCanvas].
class TextPaintService {
  TextPaintService(this.paragraph);

  final CanvasParagraph paragraph;

  void paint(BitmapCanvas canvas, ui.Offset offset) {
    // Loop through all the lines, for each line, loop through all the boxes and
    // paint them. The boxes have enough information so they can be painted
    // individually.
    final List<ParagraphLine> lines = paragraph.lines;

    if (lines.isEmpty) {
      return;
    }

    for (final ParagraphLine line in lines) {
      final List<LayoutFragment> fragments = line.fragments;
      if (fragments.isEmpty) {
        continue;
      }

      for (int i = 0; i < fragments.length; i++) {
        _paintBackground(canvas, offset, line, fragments[i]);
        _paintText(canvas, offset, line, fragments[i]);
      }
    }
  }

  void _paintBackground(
    BitmapCanvas canvas,
    ui.Offset offset,
    ParagraphLine line,
    LayoutFragment fragment,
  ) {
    final ParagraphSpan span = fragment.span;
    if (span is FlatTextSpan) {
      // Paint the background of the box, if the span has a background.
      final SurfacePaint? background = span.style.background as SurfacePaint?;
      if (background != null) {
        final ui.Rect rect = fragment.toPaintingTextBox().toRect();
        if (!rect.isEmpty) {
          canvas.drawRect(rect.shift(offset), background.paintData);
        }
      }
    }
  }

  void _paintText(
    BitmapCanvas canvas,
    ui.Offset offset,
    ParagraphLine line,
    LayoutFragment fragment,
  ) {
    // There's no text to paint in placeholder spans.
    if (fragment.isPlaceholder) {
      return;
    }

    _prepareCanvasForFragment(canvas, fragment);
    final double fragmentX = fragment.textDirection! == ui.TextDirection.ltr
        ? fragment.left
        : fragment.right;

    final double x = offset.dx + line.left + fragmentX;
    final double y = offset.dy + line.baseline;

    final EngineTextStyle style = fragment.style;

    // Don't paint the text for space-only boxes. This is just an
    // optimization, it doesn't have any effect on the output.
    if (!fragment.isSpaceOnly) {
      final String text = fragment.getText(paragraph);
      final double? letterSpacing = style.letterSpacing;
      if (letterSpacing == null || letterSpacing == 0.0) {
        canvas.drawText(text, x, y,
            style: style.foreground?.style, shadows: style.shadows);
      } else {
        // TODO(mdebbar): Implement letter-spacing on canvas more efficiently:
        //                https://github.com/flutter/flutter/issues/51234
        double charX = x;
        final int len = text.length;
        for (int i = 0; i < len; i++) {
          final String char = text[i];
          canvas.drawText(char, charX.roundToDouble(), y,
              style: style.foreground?.style,
              shadows: style.shadows);
          charX += letterSpacing + canvas.measureText(char).width!;
        }
      }
    }

    canvas.tearDownPaint();
  }

  void _prepareCanvasForFragment(BitmapCanvas canvas, LayoutFragment fragment) {
    final EngineTextStyle style = fragment.style;

    final SurfacePaint? paint;
    final ui.Paint? foreground = style.foreground;
    if (foreground != null) {
      paint = foreground as SurfacePaint;
    } else {
      paint = (ui.Paint()..color = style.color!) as SurfacePaint;
    }

    canvas.setCssFont(style.cssFontString, fragment.textDirection!);
    canvas.setUpPaint(paint.paintData, null);
  }
}
