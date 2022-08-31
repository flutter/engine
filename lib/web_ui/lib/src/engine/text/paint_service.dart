// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../html/bitmap_canvas.dart';
import '../html/painting.dart';
import 'canvas_paragraph.dart';
import 'layout_service.dart';
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
      final List<MeasuredFragment> fragments = line.fragments;
      if (fragments.isEmpty) {
        continue;
      }

      final int lengthExcludingTrailingSpaces = fragments.length - line.trailingSpaceBoxCount;

      for (int i = 0; i < lengthExcludingTrailingSpaces; i++) {
        _paintBackground(canvas, offset, line, fragments[i]);
        _paintText(canvas, offset, line, fragments[i]);
      }
      for (int i = lengthExcludingTrailingSpaces; i < fragments.length; i++) {
        _paintText(canvas, offset, line, fragments[i]);
      }
    }
  }

  void _paintBackground(
    BitmapCanvas canvas,
    ui.Offset offset,
    ParagraphLine line,
    MeasuredFragment fragment,
  ) {
    final ParagraphSpan span = fragment.span;
    if (span is FlatTextSpan) {
      // Paint the background of the box, if the span has a background.
      final SurfacePaint? background = span.style.background as SurfacePaint?;
      if (background != null) {
        final ui.Rect rect = fragment.toTextBox(line, forPainting: true).toRect().shift(offset);
        canvas.drawRect(rect, background.paintData);
      }
    }
  }

  void _paintText(
    BitmapCanvas canvas,
    ui.Offset offset,
    ParagraphLine line,
    MeasuredFragment fragment,
  ) {
    // There's no text to paint in placeholder spans.
    final ParagraphSpan span = fragment.span;
    if (span is FlatTextSpan) {
      _applySpanStyleToCanvas(span, canvas);
      final double x = offset.dx + line.left + fragment.left;
      final double y = offset.dy + line.baseline;

      // Don't paint the text for space-only boxes. This is just an
      // optimization, it doesn't have any effect on the output.
      if (!fragment.isSpaceOnly) {
        final String text = paragraph.toPlainText().substring(
              fragment.start,
              fragment.end - fragment.trailingNewlines,
            );
        final double? letterSpacing = span.style.letterSpacing;
        if (letterSpacing == null || letterSpacing == 0.0) {
          canvas.drawText(text, x, y,
              style: span.style.foreground?.style, shadows: span.style.shadows);
        } else {
          // TODO(mdebbar): Implement letter-spacing on canvas more efficiently:
          //                https://github.com/flutter/flutter/issues/51234
          double charX = x;
          final int len = text.length;
          for (int i = 0; i < len; i++) {
            final String char = text[i];
            canvas.drawText(char, charX.roundToDouble(), y,
                style: span.style.foreground?.style,
                shadows: span.style.shadows);
            charX += letterSpacing + canvas.measureText(char).width!;
          }
        }
      }

      // TODO(mdebbar): Do we need this now that we have an EllipsisFragment?

      // Paint the ellipsis using the same span styles.
      final String? ellipsis = line.ellipsis;
      if (ellipsis != null && fragment == line.fragments.last) {
        final double x = offset.dx + line.left + fragment.right;
        canvas.drawText(ellipsis, x, y, style: span.style.foreground?.style);
      }

      canvas.tearDownPaint();
    }
  }

  void _applySpanStyleToCanvas(FlatTextSpan span, BitmapCanvas canvas) {
    final SurfacePaint? paint;
    final ui.Paint? foreground = span.style.foreground;
    if (foreground != null) {
      paint = foreground as SurfacePaint;
    } else {
      paint = (ui.Paint()..color = span.style.color!) as SurfacePaint;
    }

    canvas.setCssFont(span.style.cssFontString);
    canvas.setUpPaint(paint.paintData, null);
  }
}
