// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of engine;

/// Performs layout on a [CanvasParagraph].
///
/// It uses a [html.CanvasElement] to measure text.
class TextLayoutService {
  TextLayoutService(this.paragraph);

  final CanvasParagraph paragraph;

  final html.CanvasRenderingContext2D context = html.CanvasElement().context2D;

  // *** Results of layout *** //

  // Look at the Paragraph class for documentation of the following properties.

  double width = -1.0;

  double height = 0.0;

  double longestLine = 0.0;

  double minIntrinsicWidth = 0.0;

  double maxIntrinsicWidth = 0.0;

  double alphabeticBaseline = -1.0;

  double ideographicBaseline = -1.0;

  bool didExceedMaxLines = false;

  final List<EngineLineMetrics> lines = <EngineLineMetrics>[];

  // *** Convenient shortcuts used during layout *** //

  int? get maxLines => paragraph.paragraphStyle._maxLines;
  bool get unlimitedLines => maxLines == null;
  bool get hasEllipsis => paragraph.paragraphStyle._ellipsis != null;

  void perform(ui.ParagraphConstraints constraints) {
    final int spanCount = paragraph.spans.length;

    // Reset results from previous layout.
    width = constraints.width;
    height = 0.0;
    longestLine = 0.0;
    minIntrinsicWidth = 0.0;
    maxIntrinsicWidth = 0.0;
    didExceedMaxLines = false;
    lines.clear();

    final Spanometer spanometer = Spanometer(paragraph, context);

    int spanIndex = 0;
    ParagraphSpan span = paragraph.spans[0];
    LineBuilder currentLine = LineBuilder.first(paragraph, spanometer);
    LineBuilder maxIntrinsicLine = LineBuilder.first(paragraph, spanometer);

    // The only way to exit this while loop is by hitting the `break;` statement
    // when we reach the `endOfText` line break.
    while (true) {

      // *********************************************** //
      // *** HANDLE HARD LINE BREAKS AND END OF TEXT *** //
      // *********************************************** //

      if (currentLine.end.isHard) {
        if (currentLine.isNotEmpty) {
          lines.add(currentLine.build());
        }

        if (currentLine.end.type == LineBreakType.endOfText) {
          break;
        } else {
          currentLine = currentLine.nextLine();
        }
      }

      // ********************************* //
      // *** THE MAIN MEASUREMENT PART *** //
      // ********************************* //

      if (span is PlaceholderSpan) {
        spanometer.currentSpan = null;
        final double lineWidth = currentLine.width + span.width;
        // TODO(mdebbar): Consider how placeholders affect min/max intrinsics.
        if (lineWidth <= constraints.width) {
          // The placeholder fits on the current line.
          // TODO(mdebbar):
          // (1) adjust the current line's height to fit the placeholder.
          // (2) update accumulated line width.
        } else {
          // The placeholder can't fit on the current line.
          // TODO(mdebbar):
          // (1) create a line.
          // (2) adjust the new line's height to fit the placeholder.
          // (3) update `lineStart`, etc.
        }
      } else if (span is FlatTextSpan) {
        spanometer.currentSpan = span;
        final LineBreakResult nextBreak = currentLine.findNextBreak(span.end);
        final double additionalWidth =
            currentLine.getAdditionalWidthTo(nextBreak);

        // For the purpose of max intrinsic width, we don't care if the line
        // fits within the constraints or not. So we always extend it.
        if (maxIntrinsicLine.end != nextBreak) {
          maxIntrinsicLine.extendTo(nextBreak);
        }

        if (currentLine.width + additionalWidth <= constraints.width) {
          // TODO(mdebbar): Handle the case when `nextBreak` is just a span end
          //                that shouldn't extend the line yet.

          // The line can extend to `nextBreak` without overflowing.
          currentLine.extendTo(nextBreak);
        } else {
          // The chunk of text can't fit into the current line.
          final bool isLastLine =
              (hasEllipsis && unlimitedLines) || lines.length + 1 == maxLines;
          if (isLastLine && hasEllipsis) {
            // We've reached the line that requires an ellipsis to be appended
            // to it.

            // TODO(mdebbar): Remove this line and implement overflow ellipsis.
            currentLine.extendTo(nextBreak);
          } else if (currentLine.isEmpty) {
            // The current line is still empty, which means we are dealing
            // with a single block of text that doesn't fit in a single line.
            // We need to force-break it.

            // TODO(mdebbar): Remove this line and implement force-breaking.
            currentLine.extendTo(nextBreak);
          } else {
            // Normal line break.
            lines.add(currentLine.build());
            currentLine = currentLine.nextLine();
          }
        }
      } else {
        throw UnimplementedError('Unknown span type: ${span.runtimeType}');
      }

      // ************************************************ //
      // *** LONGEST LINE && MAX/MIN INTRINSIC WIDTHS *** //
      // ************************************************ //

      if (longestLine < currentLine.width) {
        longestLine = currentLine.width;
      }

      if (minIntrinsicWidth < currentLine.widthOfLastExtension) {
        minIntrinsicWidth = currentLine.widthOfLastExtension;
      }

      if (maxIntrinsicLine.end.isHard) {
        // Max intrinsic width includes the width of trailing spaces.
        if (maxIntrinsicWidth < maxIntrinsicLine.widthIncludingSpace) {
          maxIntrinsicWidth = maxIntrinsicLine.widthIncludingSpace;
        }
        maxIntrinsicLine = maxIntrinsicLine.nextLine();
      }

      // ********************************************* //
      // *** ADVANCE TO THE NEXT SPAN IF NECESSARY *** //
      // ********************************************* //

      // Only go to the next span if we've reached the end of this span.
      if (currentLine.end.index >= span.end && spanIndex < spanCount - 1) {
        span = paragraph.spans[++spanIndex];
      }
    }
  }
}

class LineBuilder {
  LineBuilder._(
    this.paragraph,
    this.spanometer, {
    required this.start,
    required this.lineNumber,
  }) : end = start;

  factory LineBuilder.first(CanvasParagraph paragraph, Spanometer spanometer) {
    return LineBuilder._(
      paragraph,
      spanometer,
      lineNumber: 0,
      start: LineBreakResult.sameIndex(0, LineBreakType.prohibited),
    );
  }

  final CanvasParagraph paragraph;
  final Spanometer spanometer;
  final LineBreakResult start;
  final int lineNumber;

  LineBreakResult end;

  double width = 0.0;
  double widthOfTrailingSpace = 0.0;
  double get widthIncludingSpace => width + widthOfTrailingSpace;
  double widthOfLastExtension = 0.0;

  bool get isEmpty => start == end;
  bool get isNotEmpty => !isEmpty;

  double getAdditionalWidthTo(LineBreakResult newEnd) {
    // If the extension is all made of space characters, it shouldn't add
    // anything to the width.
    if (end.index == newEnd.indexWithoutTrailingSpaces) {
      return 0.0;
    }

    return widthOfTrailingSpace + spanometer.measure(end, newEnd);
  }

  void extendTo(LineBreakResult newEnd) {
    // If the current end of the line is a hard break, the line shouldn't be
    // extended any further.
    assert(
      isEmpty || !end.isHard,
      'Cannot extend a line that ends with a hard break.',
    );

    // TODO(mdebbar): Handle the case where the entire extension is made of spaces.
    widthOfLastExtension = spanometer.measure(end, newEnd);
    final double additionalWidthIncludingSpace =
        spanometer.measureIncludingSpace(end, newEnd);

    // Add the width of previous trailing space.
    width += widthOfTrailingSpace + widthOfLastExtension;
    widthOfTrailingSpace = additionalWidthIncludingSpace - widthOfLastExtension;
    end = newEnd;
  }

  EngineLineMetrics build() {
    final String text = paragraph.toPlainText();
    return EngineLineMetrics.withText(
      text.substring(start.index, end.indexWithoutTrailingNewlines),
      startIndex: start.index,
      endIndex: end.index,
      endIndexWithoutNewlines: end.indexWithoutTrailingNewlines,
      hardBreak: end.isHard,
      width: width,
      widthWithTrailingSpaces: width + widthOfTrailingSpace,
      // TODO(mdebbar): Calculate actual align offset.
      left: 0.0,
      lineNumber: lineNumber,
    );
  }

  LineBreakResult findNextBreak(int maxEnd) {
    return nextLineBreak(paragraph.toPlainText(), end.index, maxEnd: maxEnd);
  }

  LineBuilder nextLine() {
    return LineBuilder._(
      paragraph,
      spanometer,
      start: end,
      lineNumber: lineNumber + 1,
    );
  }
}

class Spanometer {
  Spanometer(this.paragraph, this.context);

  final CanvasParagraph paragraph;
  final html.CanvasRenderingContext2D context;

  String _cssFontString = '';

  double? get letterSpacing => _currentSpan!.style._letterSpacing;

  FlatTextSpan? _currentSpan;
  set currentSpan(FlatTextSpan? span) {
    if (span == _currentSpan) {
      return;
    }
    _currentSpan = span;

    // No need to update css font string when `span` is null.
    if (span == null) {
      return;
    }

    // Update the font string if it's different from the previous span.
    final String cssFontString = span.style.cssFontString;
    if (_cssFontString != cssFontString) {
      _cssFontString = cssFontString;
      context.font = cssFontString;
    }
  }

  double measure(LineBreakResult start, LineBreakResult end) {
    return _measure(start.index, end.indexWithoutTrailingSpaces);
  }

  double measureIncludingSpace(LineBreakResult start, LineBreakResult end) {
    return _measure(start.index, end.indexWithoutTrailingNewlines);
  }

  double _measure(int start, int end) {
    assert(_currentSpan != null);
    final FlatTextSpan span = _currentSpan!;

    // Make sure the range is within the current span.
    assert(start >= span.start && start <= span.end);
    assert(end >= span.start && end <= span.end);

    final String text = paragraph.toPlainText();
    return _measureSubstring(
      context,
      text,
      start,
      end,
      letterSpacing: letterSpacing,
    );
  }
}
