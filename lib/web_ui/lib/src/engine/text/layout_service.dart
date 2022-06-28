// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;

import 'package:meta/meta.dart';
import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import 'canvas_paragraph.dart';
import 'fragmenter.dart';
import 'line_breaker.dart';
import 'measurement.dart';
import 'paragraph.dart';
import 'ruler.dart';

/// Performs layout on a [CanvasParagraph].
///
/// It uses a [DomCanvasElement] to measure text.
class TextLayoutService {
  TextLayoutService(this.paragraph);

  final CanvasParagraph paragraph;

  final DomCanvasRenderingContext2D context =
      createDomCanvasElement().context2D;

  // *** Results of layout *** //

  // Look at the Paragraph class for documentation of the following properties.

  double width = -1.0;

  double height = 0.0;

  ParagraphLine? longestLine;

  double minIntrinsicWidth = 0.0;

  double maxIntrinsicWidth = 0.0;

  double alphabeticBaseline = -1.0;

  double ideographicBaseline = -1.0;

  bool didExceedMaxLines = false;

  final List<ParagraphLine> lines = <ParagraphLine>[];

  /// The bounds that contain the text painted inside this paragraph.
  ui.Rect get paintBounds => _paintBounds;
  ui.Rect _paintBounds = ui.Rect.zero;

  // *** Convenient shortcuts used during layout *** //

  int? get maxLines => paragraph.paragraphStyle.maxLines;
  bool get unlimitedLines => maxLines == null;

  String? get ellipsis => paragraph.paragraphStyle.ellipsis;
  bool get hasEllipsis => ellipsis != null;

  late final Spanometer spanometer = Spanometer(paragraph, context);

  late final List<LayoutFragment> fragments = LayoutFragmenter(paragraph).fragment();

  late final Iterable<MeasuredFragment> measuredFragments = fragments.map(spanometer.measureFragment);

  /// Performs the layout on a paragraph given the [constraints].
  ///
  /// The function starts by resetting all layout-related properties. Then it
  /// starts looping through the paragraph to calculate all layout metrics.
  ///
  /// It uses a [Spanometer] to perform measurements within spans of the
  /// paragraph. It also uses [LineBuilders] to generate [ParagraphLine]s as
  /// it iterates through the paragraph.
  ///
  /// The main loop keeps going until:
  ///
  /// 1. The end of the paragraph is reached (i.e. LineBreakType.endOfText).
  /// 2. Enough lines have been computed to satisfy [maxLines].
  /// 3. An ellipsis is appended because of an overflow.
  void performLayout(ui.ParagraphConstraints constraints) {
    final int spanCount = paragraph.spans.length;

    // Reset results from previous layout.
    width = constraints.width;
    height = 0.0;
    longestLine = null;
    minIntrinsicWidth = 0.0;
    maxIntrinsicWidth = 0.0;
    didExceedMaxLines = false;
    lines.clear();

    if (spanCount == 0) {
      return;
    }

    LineBuilder currentLine =
        LineBuilder.first(paragraph, spanometer, maxWidth: constraints.width);

    for (final MeasuredFragment fragment in measuredFragments) {
      if (currentLine.canAddFragment(fragment)) {
        // The line can still fit the new fragment.

        currentLine.addFragment(fragment);
      } else if (currentLine.canHaveEllipsis) {
        // We should stop at this line and add the ellipsis.

        currentLine.addFragment(fragment);
        // TODO(mdebbar): this should automatically force-break the line to fit the ellipsis.
        currentLine.insertEllipsis();
        lines.add(currentLine.build());
        // TODO(mdebbar): didExceedMaxLines = true ?
        break;
      } else if (currentLine.isNotBreakable) {
        // The line is too long and needs to be force-broken.

        // Add only part of the fragment that can fit in the line.
        currentLine.addPartOfFragment(fragment);
        lines.add(currentLine.build());
        // TODO(mdebbar): didExceedMaxLines = true ?
        break;
      } else {
        // Normal line break.

        // TODO(mdebbar): `LineBuilder.build` should not include trailing non-breakable fragments.
        lines.add(currentLine.build());
        // TODO(mdebbar): `LineBuilder.nextLine` should include all trailing non-breakable fragments.
        currentLine = currentLine.nextLine();
      }
    }

    // ***************************************************************** //
    // *** PARAGRAPH BASELINE & HEIGHT & LONGEST LINE & PAINT BOUNDS *** //
    // ***************************************************************** //

    double boundsLeft = double.infinity;
    double boundsRight = double.negativeInfinity;
    for (final ParagraphLine line in lines) {
      height += line.height;
      if (alphabeticBaseline == -1.0) {
        alphabeticBaseline = line.baseline;
        ideographicBaseline = alphabeticBaseline * baselineRatioHack;
      }
      final double longestLineWidth = longestLine?.width ?? 0.0;
      if (longestLineWidth < line.width) {
        longestLine = line;
      }

      final double left = line.left;
      if (left < boundsLeft) {
        boundsLeft = left;
      }
      final double right = left + line.width;
      if (right > boundsRight) {
        boundsRight = right;
      }
    }
    _paintBounds = ui.Rect.fromLTRB(
      boundsLeft,
      0,
      boundsRight,
      height,
    );

    // ************************** //
    // *** POSITION FRAGMENTS *** //
    // ************************** //

    if (lines.isNotEmpty) {
      final ParagraphLine lastLine = lines.last;
      final bool shouldJustifyParagraph = width.isFinite &&
          paragraph.paragraphStyle.textAlign == ui.TextAlign.justify;

      for (final ParagraphLine line in lines) {
        // Don't apply justification to the last line.
        final bool shouldJustifyLine =
            shouldJustifyParagraph && line != lastLine;
        _positionLineBoxes(line, withJustification: shouldJustifyLine);
      }
    }

    // ******************************** //
    // *** MAX/MIN INTRINSIC WIDTHS *** //
    // ******************************** //

    double runningMinIntrinsicWidth = 0;
    double runningMaxIntrinsicWidth = 0;

    for (final MeasuredFragment fragment in measuredFragments) {
      runningMinIntrinsicWidth += fragment.metrics.widthExcludingTrailingSpaces;
      // Max intrinsic width includes the width of trailing spaces.
      runningMaxIntrinsicWidth += fragment.metrics.widthIncludingTrailingSpaces;

      switch (fragment.type) {
        case LineBreakType.prohibited:
          break;

        case LineBreakType.opportunity:
          minIntrinsicWidth = math.max(minIntrinsicWidth, runningMinIntrinsicWidth);
          runningMinIntrinsicWidth = 0;
          break;

        case LineBreakType.mandatory:
        case LineBreakType.endOfText:
          minIntrinsicWidth = math.max(minIntrinsicWidth, runningMinIntrinsicWidth);
          maxIntrinsicWidth = math.max(maxIntrinsicWidth, runningMaxIntrinsicWidth);
          runningMinIntrinsicWidth = 0;
          runningMaxIntrinsicWidth = 0;
          break;
      }
    }
  }

  ui.TextDirection get _paragraphDirection =>
      paragraph.paragraphStyle.effectiveTextDirection;

  /// Positions the boxes in the given [line] and takes into account their
  /// directions, the paragraph's direction, and alignment justification.
  void _positionLineBoxes(
    ParagraphLine line, {
    required bool withJustification,
  }) {
    final List<RangeBox> boxes = line.boxes;
    final double justifyPerSpaceBox =
        withJustification ? _calculateJustifyPerSpaceBox(line) : 0.0;

    int i = 0;
    double cumulativeWidth = 0.0;
    while (i < boxes.length) {
      final RangeBox box = boxes[i];
      if (box.boxDirection == _paragraphDirection) {
        // The box is in the same direction as the paragraph.
        box.startOffset = cumulativeWidth;
        box.lineWidth = line.width;
        if (box is SpanBox && box.isSpaceOnly && !box.isTrailingSpace) {
          box._width += justifyPerSpaceBox;
        }

        cumulativeWidth += box.width;
        i++;
        continue;
      }

      // At this point, we found a box that has the opposite direction to the
      // paragraph. This could be a sequence of one or more boxes.
      //
      // These boxes should flow in the opposite direction. So we need to
      // position them in reverse order.
      //
      // If the last box in the sequence is a space-only box (contains only
      // whitespace characters), it should be excluded from the sequence.
      //
      // Example: an LTR paragraph with the contents:
      //
      // "ABC rtl1 rtl2 rtl3 XYZ"
      //     ^    ^    ^    ^
      //    SP1  SP2  SP3  SP4
      //
      //
      // box direction:    LTR           RTL               LTR
      //                |------>|<-----------------------|------>
      //                +----------------------------------------+
      //                | ABC | | rtl3 | | rtl2 | | rtl1 | | XYZ |
      //                +----------------------------------------+
      //                       ^        ^        ^        ^
      //                      SP1      SP3      SP2      SP4
      //
      // Notice how SP2 and SP3 are flowing in the RTL direction because of the
      // surrounding RTL words. SP4 is also preceded by an RTL word, but it marks
      // the end of the RTL sequence, so it goes back to flowing in the paragraph
      // direction (LTR).

      final int first = i;
      int lastNonSpaceBox = first;
      i++;
      while (i < boxes.length && boxes[i].boxDirection != _paragraphDirection) {
        final RangeBox box = boxes[i];
        if (box is SpanBox && box.isSpaceOnly) {
          // Do nothing.
        } else {
          lastNonSpaceBox = i;
        }
        i++;
      }
      final int last = lastNonSpaceBox;
      i = lastNonSpaceBox + 1;

      // The range (first:last) is the entire sequence of boxes that have the
      // opposite direction to the paragraph.
      final double sequenceWidth = _positionLineBoxesInReverse(
        line,
        first,
        last,
        startOffset: cumulativeWidth,
        justifyPerSpaceBox: justifyPerSpaceBox,
      );
      cumulativeWidth += sequenceWidth;
    }
  }

  /// Positions a sequence of boxes in the direction opposite to the paragraph
  /// text direction.
  ///
  /// This is needed when a right-to-left sequence appears in the middle of a
  /// left-to-right paragraph, or vice versa.
  ///
  /// Returns the total width of all the positioned boxes in the sequence.
  ///
  /// [first] and [last] are expected to be inclusive.
  double _positionLineBoxesInReverse(
    ParagraphLine line,
    int first,
    int last, {
    required double startOffset,
    required double justifyPerSpaceBox,
  }) {
    final List<RangeBox> boxes = line.boxes;
    double cumulativeWidth = 0.0;
    for (int i = last; i >= first; i--) {
      // Update the visual position of each box.
      final RangeBox box = boxes[i];
      assert(box.boxDirection != _paragraphDirection);
      box.startOffset = startOffset + cumulativeWidth;
      box.lineWidth = line.width;
      if (box is SpanBox && box.isSpaceOnly && !box.isTrailingSpace) {
        box._width += justifyPerSpaceBox;
      }

      cumulativeWidth += box.width;
    }
    return cumulativeWidth;
  }

  /// Calculates for the given [line], the amount of extra width that needs to be
  /// added to each space box in order to align the line with the rest of the
  /// paragraph.
  double _calculateJustifyPerSpaceBox(ParagraphLine line) {
    final double justifyTotal = width - line.width;

    final int spaceBoxesToJustify = line.nonTrailingSpaceBoxCount;
    if (spaceBoxesToJustify > 0) {
      return justifyTotal / spaceBoxesToJustify;
    }

    return 0.0;
  }

  List<ui.TextBox> getBoxesForPlaceholders() {
    final List<ui.TextBox> boxes = <ui.TextBox>[];
    for (final ParagraphLine line in lines) {
      for (final RangeBox box in line.boxes) {
        if (box is PlaceholderBox) {
          boxes.add(box.toTextBox(line, forPainting: false));
        }
      }
    }
    return boxes;
  }

  List<ui.TextBox> getBoxesForRange(
    int start,
    int end,
    ui.BoxHeightStyle boxHeightStyle,
    ui.BoxWidthStyle boxWidthStyle,
  ) {
    // Zero-length ranges and invalid ranges return an empty list.
    if (start >= end || start < 0 || end < 0) {
      return <ui.TextBox>[];
    }

    final int length = paragraph.toPlainText().length;
    // Ranges that are out of bounds should return an empty list.
    if (start > length || end > length) {
      return <ui.TextBox>[];
    }

    final List<ui.TextBox> boxes = <ui.TextBox>[];

    for (final ParagraphLine line in lines) {
      if (line.overlapsWith(start, end)) {
        for (final RangeBox box in line.boxes) {
          if (box is SpanBox && box.overlapsWith(start, end)) {
            boxes.add(box.intersect(line, start, end, forPainting: false));
          }
        }
      }
    }
    return boxes;
  }

  ui.TextPosition getPositionForOffset(ui.Offset offset) {
    // After layout, each line has boxes that contain enough information to make
    // it possible to do hit testing. Once we find the box, we look inside that
    // box to find where exactly the `offset` is located.

    final ParagraphLine line = _findLineForY(offset.dy);
    // [offset] is to the left of the line.
    if (offset.dx <= line.left) {
      return ui.TextPosition(
        offset: line.startIndex,
        affinity: ui.TextAffinity.downstream,
      );
    }

    // [offset] is to the right of the line.
    if (offset.dx >= line.left + line.widthWithTrailingSpaces) {
      return ui.TextPosition(
        offset: line.endIndexWithoutNewlines,
        affinity: ui.TextAffinity.upstream,
      );
    }

    final double dx = offset.dx - line.left;
    for (final RangeBox box in line.boxes) {
      if (box.left <= dx && dx <= box.right) {
        return box.getPositionForX(dx);
      }
    }
    // Is this ever reachable?
    return ui.TextPosition(offset: line.startIndex);
  }

  ParagraphLine _findLineForY(double y) {
    // We could do a binary search here but it's not worth it because the number
    // of line is typically low, and each iteration is a cheap comparison of
    // doubles.
    for (final ParagraphLine line in lines) {
      if (y <= line.height) {
        return line;
      }
      y -= line.height;
    }
    return lines.last;
  }
}

/// Represents a box inside a paragraph span with the range of [start] to [end].
///
/// The box's coordinates are all relative to the line it belongs to. For
/// example, [left] is the distance from the left edge of the line to the left
/// edge of the box.
///
/// This is what the various measurements/coordinates look like for a box in an
/// LTR paragraph:
///
///          *------------------------lineWidth------------------*
///                            *--width--*
///          ┌─────────────────┬─────────┬───────────────────────┐
///          │                 │---BOX---│                       │
///          └─────────────────┴─────────┴───────────────────────┘
///          *---startOffset---*
///          *------left-------*
///          *--------endOffset----------*
///          *----------right------------*
///
///
/// And in an RTL paragraph, [startOffset] and [endOffset] are flipped because
/// the line starts from the right. Here's what they look like:
///
///          *------------------------lineWidth------------------*
///                            *--width--*
///          ┌─────────────────┬─────────┬───────────────────────┐
///          │                 │---BOX---│                       │
///          └─────────────────┴─────────┴───────────────────────┘
///                                      *------startOffset------*
///          *------left-------*
///                            *-----------endOffset-------------*
///          *----------right------------*
///
abstract class RangeBox {
  RangeBox(
    this.start,
    this.end,
    this.paragraphDirection,
    this.boxDirection,
  );

  final LineBreakResult start;
  final LineBreakResult end;

  /// The distance from the beginning of the line to the beginning of the box.
  late final double startOffset;

  /// The distance from the beginning of the line to the end of the box.
  double get endOffset => startOffset + width;

  /// The distance from the left edge of the line to the left edge of the box.
  double get left => paragraphDirection == ui.TextDirection.ltr
      ? startOffset
      : lineWidth - endOffset;

  /// The distance from the left edge of the line to the right edge of the box.
  double get right => paragraphDirection == ui.TextDirection.ltr
      ? endOffset
      : lineWidth - startOffset;

  /// The distance from the left edge of the box to the right edge of the box.
  double get width;

  /// The width of the line that this box belongs to.
  late final double lineWidth;

  /// The text direction of the paragraph that this box belongs to.
  final ui.TextDirection paragraphDirection;

  /// Indicates how this box flows among other boxes.
  ///
  /// Example: In an LTR paragraph, the text "ABC hebrew_word 123 DEF" is shown
  /// visually in the following order:
  ///
  ///                +-------------------------------+
  ///                | ABC | 123 | drow_werbeh | DEF |
  ///                +-------------------------------+
  /// box direction:   LTR   RTL       RTL       LTR
  ///                 ----> <---- <------------  ---->
  ///
  /// (In the above example, we are ignoring whitespace to simplify).
  final ui.TextDirection boxDirection;

  /// Returns a [ui.TextBox] representing this range box in the given [line].
  ///
  /// The coordinates of the resulting [ui.TextBox] are relative to the
  /// paragraph, not to the line.
  ///
  /// The [forPainting] parameter specifies whether the text box is wanted for
  /// painting purposes or not. The difference is observed in the handling of
  /// trailing spaces. Trailing spaces aren't painted on the screen, but their
  /// dimensions are still useful for other cases like highlighting selection.
  ui.TextBox toTextBox(ParagraphLine line, {required bool forPainting});

  /// Returns the text position within this box's range that's closest to the
  /// given [x] offset.
  ///
  /// The [x] offset is expected to be relative to the left edge of the line,
  /// just like the coordinates of this box.
  ui.TextPosition getPositionForX(double x);
}

/// Represents a box for a [PlaceholderSpan].
class PlaceholderBox extends RangeBox {
  PlaceholderBox(
    this.placeholder, {
    required LineBreakResult index,
    required ui.TextDirection paragraphDirection,
    required ui.TextDirection boxDirection,
  }) : super(index, index, paragraphDirection, boxDirection);

  final PlaceholderSpan placeholder;

  @override
  double get width => placeholder.width;

  @override
  ui.TextBox toTextBox(ParagraphLine line, {required bool forPainting}) {
    final double left = line.left + this.left;
    final double right = line.left + this.right;

    final double lineTop = line.baseline - line.ascent;

    final double top;
    switch (placeholder.alignment) {
      case ui.PlaceholderAlignment.top:
        top = lineTop;
        break;

      case ui.PlaceholderAlignment.middle:
        top = lineTop + (line.height - placeholder.height) / 2;
        break;

      case ui.PlaceholderAlignment.bottom:
        top = lineTop + line.height - placeholder.height;
        break;

      case ui.PlaceholderAlignment.aboveBaseline:
        top = line.baseline - placeholder.height;
        break;

      case ui.PlaceholderAlignment.belowBaseline:
        top = line.baseline;
        break;

      case ui.PlaceholderAlignment.baseline:
        top = line.baseline - placeholder.baselineOffset;
        break;
    }

    return ui.TextBox.fromLTRBD(
      left,
      top,
      right,
      top + placeholder.height,
      paragraphDirection,
    );
  }

  @override
  ui.TextPosition getPositionForX(double x) {
    // See if `x` is closer to the left edge or the right edge of the box.
    final bool closerToLeft = x - left < right - x;
    return ui.TextPosition(
      offset: start.index,
      affinity:
          closerToLeft ? ui.TextAffinity.upstream : ui.TextAffinity.downstream,
    );
  }
}

/// Represents a box in a [FlatTextSpan].
class SpanBox extends RangeBox {
  SpanBox(
    this.spanometer, {
    required LineBreakResult start,
    required LineBreakResult end,
    required double width,
    required ui.TextDirection paragraphDirection,
    required ui.TextDirection boxDirection,
    required this.contentDirection,
    required this.isSpaceOnly,
  })  : span = spanometer.currentSpan,
        height = spanometer.height,
        baseline = spanometer.ascent,
        _width = width,
        super(start, end, paragraphDirection, boxDirection);

  final Spanometer spanometer;
  final FlatTextSpan span;

  /// The direction of the text inside this box.
  ///
  /// To illustrate the difference between [boxDirection] and [contentDirection]
  /// here's an example:
  ///
  /// In an LTR paragraph, the text "ABC hebrew_word 123 DEF" is rendered as
  /// follows:
  ///
  ///                     ----> <---- <------------  ---->
  ///     box direction:   LTR   RTL       RTL       LTR
  ///                    +-------------------------------+
  ///                    | ABC | 123 | drow_werbeh | DEF |
  ///                    +-------------------------------+
  /// content direction:   LTR   LTR       RTL       LTR
  ///                     ----> ----> <------------  ---->
  ///
  /// Notice the box containing "123" flows in the RTL direction (because it
  /// comes after an RTL box), while the content of the box flows in the LTR
  /// direction (i.e. the text is shown as "123" not "321").
  final ui.TextDirection contentDirection;

  /// Whether this box is made of only white space.
  final bool isSpaceOnly;

  /// Whether this box is a trailing space box at the end of a line.
  bool get isTrailingSpace => _isTrailingSpace;
  bool _isTrailingSpace = false;

  /// This is made mutable so it can be updated later in the layout process for
  /// the purpose of aligning the lines of a paragraph with [ui.TextAlign.justify].
  double _width;

  @override
  double get width => _width;

  /// Whether the contents of this box flow in the left-to-right direction.
  bool get isContentLtr => contentDirection == ui.TextDirection.ltr;

  /// Whether the contents of this box flow in the right-to-left direction.
  bool get isContentRtl => !isContentLtr;

  /// The distance from the top edge to the bottom edge of the box.
  final double height;

  /// The distance from the top edge of the box to the alphabetic baseline of
  /// the box.
  final double baseline;

  /// Whether this box's range overlaps with the range from [startIndex] to
  /// [endIndex].
  bool overlapsWith(int startIndex, int endIndex) {
    return startIndex < end.index && start.index < endIndex;
  }

  /// Returns the substring of the paragraph that's represented by this box.
  ///
  /// Trailing newlines are omitted, if any.
  String toText() {
    return spanometer.paragraph
        .toPlainText()
        .substring(start.index, end.indexWithoutTrailingNewlines);
  }

  @override
  ui.TextBox toTextBox(ParagraphLine line, {required bool forPainting}) {
    return intersect(line, start.index, end.index, forPainting: forPainting);
  }

  /// Performs the intersection of this box with the range given by [start] and
  /// [end] indices, and returns a [ui.TextBox] representing that intersection.
  ///
  /// The coordinates of the resulting [ui.TextBox] are relative to the
  /// paragraph, not to the line.
  ui.TextBox intersect(ParagraphLine line, int start, int end,
      {required bool forPainting}) {
    final double top = line.baseline - baseline;

    final double before;
    if (start <= this.start.index) {
      before = 0.0;
    } else {
      spanometer.currentSpan = span;
      before = spanometer._measure(this.start.index, start);
    }

    final double after;
    if (end >= this.end.indexWithoutTrailingNewlines) {
      after = 0.0;
    } else {
      spanometer.currentSpan = span;
      after = spanometer._measure(end, this.end.indexWithoutTrailingNewlines);
    }

    double left, right;
    if (isContentLtr) {
      // Example: let's say the text is "Loremipsum" and we want to get the box
      // for "rem". In this case, `before` is the width of "Lo", and `after`
      // is the width of "ipsum".
      //
      // Here's how the measurements/coordinates look like:
      //
      //              before         after
      //              |----|     |----------|
      //              +---------------------+
      //              | L o r e m i p s u m |
      //              +---------------------+
      //    this.left ^                     ^ this.right
      left = this.left + before;
      right = this.right - after;
    } else {
      // Example: let's say the text is "txet_werbeH" ("Hebrew_text" flowing from
      // right to left). Say we want to get the box for "brew". The `before` is
      // the width of "He", and `after` is the width of "_text".
      //
      //                 after           before
      //              |----------|       |----|
      //              +-----------------------+
      //              | t x e t _ w e r b e H |
      //              +-----------------------+
      //    this.left ^                       ^ this.right
      //
      // Notice how `before` and `after` are reversed in the RTL example. That's
      // because the text flows from right to left.
      left = this.left + after;
      right = this.right - before;
    }

    // When painting a paragraph, trailing spaces should have a zero width.
    final bool isZeroWidth = forPainting && isTrailingSpace;
    if (isZeroWidth) {
      // Collapse the box to the left or to the right depending on the paragraph
      // direction.
      if (paragraphDirection == ui.TextDirection.ltr) {
        right = left;
      } else {
        left = right;
      }
    }

    // The [RangeBox]'s left and right edges are relative to the line. In order
    // to make them relative to the paragraph, we need to add the left edge of
    // the line.
    return ui.TextBox.fromLTRBD(
      line.left + left,
      top,
      line.left + right,
      top + height,
      contentDirection,
    );
  }

  /// Transforms the [x] coordinate to be relative to this box and matches the
  /// flow of content.
  ///
  /// In LTR paragraphs, the [startOffset] and [endOffset] of an RTL box
  /// indicate the visual beginning and end of the box. But the text inside the
  /// box flows in the opposite direction (from [endOffset] to [startOffset]).
  ///
  /// The X (input) is relative to the line, and always from left-to-right
  /// independent of paragraph and content direction.
  ///
  /// Here's how it looks for a box with LTR content:
  ///
  ///          *------------------------lineWidth------------------*
  ///          *---------------X (input)
  ///          ┌───────────┬────────────────────────┬───────────────┐
  ///          │           │ --content-direction--> │               │
  ///          └───────────┴────────────────────────┴───────────────┘
  ///                      *---X' (output)
  ///          *---left----*
  ///          *---------------right----------------*
  ///
  ///
  /// And here's how it looks for a box with RTL content:
  ///
  ///          *------------------------lineWidth------------------*
  ///          *----------------X (input)
  ///          ┌───────────┬────────────────────────┬───────────────┐
  ///          │           │ <--content-direction-- │               │
  ///          └───────────┴────────────────────────┴───────────────┘
  ///                  (output) X'------------------*
  ///          *---left----*
  ///          *---------------right----------------*
  ///
  double _makeXRelativeToContent(double x) {
    return isContentRtl ? right - x : x - left;
  }

  @override
  ui.TextPosition getPositionForX(double x) {
    spanometer.currentSpan = span;

    x = _makeXRelativeToContent(x);

    final int startIndex = start.index;
    final int endIndex = end.indexWithoutTrailingNewlines;
    // The resulting `cutoff` is the index of the character where the `x` offset
    // falls. We should return the text position of either `cutoff` or
    // `cutoff + 1` depending on which one `x` is closer to.
    //
    //   offset x
    //      ↓
    // "A B C D E F"
    //     ↑
    //   cutoff
    final int cutoff = spanometer.forceBreak(
      startIndex,
      endIndex,
      availableWidth: x,
      allowEmpty: true,
    );

    if (cutoff == endIndex) {
      return ui.TextPosition(
        offset: cutoff,
        affinity: ui.TextAffinity.upstream,
      );
    }

    final double lowWidth = spanometer._measure(startIndex, cutoff);
    final double highWidth = spanometer._measure(startIndex, cutoff + 1);

    // See if `x` is closer to `cutoff` or `cutoff + 1`.
    if (x - lowWidth < highWidth - x) {
      // The offset is closer to cutoff.
      return ui.TextPosition(
        offset: cutoff,
        affinity: ui.TextAffinity.downstream,
      );
    } else {
      // The offset is closer to cutoff + 1.
      return ui.TextPosition(
        offset: cutoff + 1,
        affinity: ui.TextAffinity.upstream,
      );
    }
  }
}

/// Builds instances of [ParagraphLine] for the given [paragraph].
///
/// Usage of this class starts by calling [LineBuilder.first] to start building
/// the first line of the paragraph.
///
/// Then fragments can be added by calling [addFragment].
///
/// Before adding a fragment, the method [canAddFragment] helps determine
/// whether the line can fit a certain fragment or not.
///
/// Once the line is complete, it can be built by calling [build] that generates
/// a [ParagraphLine] instance.
///
/// To start building the next line, simply call [nextLine] to get a new
/// [LineBuilder] for the next line.
class LineBuilder {
  LineBuilder._(
    this.paragraph,
    this.spanometer, {
    required this.maxWidth,
    required this.lineNumber,
    required this.accumulatedHeight,
    required List<MeasuredFragment> fragments,
  }) : _fragments = fragments {
    _recalculateMetrics();
  }

  /// Creates a [LineBuilder] for the first line in a paragraph.
  factory LineBuilder.first(
    CanvasParagraph paragraph,
    Spanometer spanometer, {
    required double maxWidth,
  }) {
    return LineBuilder._(
      paragraph,
      spanometer,
      maxWidth: maxWidth,
      lineNumber: 0,
      accumulatedHeight: 0.0,
      fragments: <MeasuredFragment>[],
    );
  }

  final List<MeasuredFragment> _fragments;
  List<MeasuredFragment>? _fragmentsForNextLine;

  int _lastBreakableFragment = -1;

  final List<RangeBox> _boxes = <RangeBox>[];

  final double maxWidth;
  final CanvasParagraph paragraph;
  final Spanometer spanometer;
  final int lineNumber;

  /// The accumulated height of all preceding lines, excluding the current line.
  final double accumulatedHeight;

  /// The width of the line so far, excluding trailing white space.
  double width = 0.0;

  /// The width of the line so far, including trailing white space.
  double widthIncludingSpace = 0.0;

  /// The width of trailing white space in the line.
  double get widthOfTrailingSpace => widthIncludingSpace - width;

  /// The distance from the top of the line to the alphabetic baseline.
  double ascent = 0.0;

  /// The distance from the bottom of the line to the alphabetic baseline.
  double descent = 0.0;

  /// The height of the line so far.
  double get height => ascent + descent;

  /// Returns true if there is at least one break opportunity in the line.
  bool isBreakable = false;

  /// Returns true if there's no break opportunity in the line.
  bool get isNotBreakable => !isBreakable;

  int _spaceBoxCount = 0;

  bool get isEmpty => _fragments.isEmpty;
  bool get isNotEmpty => _fragments.isNotEmpty;

  /// The horizontal offset necessary for the line to be correctly aligned.
  double get alignOffset {
    final double emptySpace = maxWidth - width;
    final ui.TextAlign textAlign = paragraph.paragraphStyle.effectiveTextAlign;

    switch (textAlign) {
      case ui.TextAlign.center:
        return emptySpace / 2.0;
      case ui.TextAlign.right:
        return emptySpace;
      case ui.TextAlign.start:
        return _paragraphDirection == ui.TextDirection.rtl ? emptySpace : 0.0;
      case ui.TextAlign.end:
        return _paragraphDirection == ui.TextDirection.rtl ? 0.0 : emptySpace;
      default:
        return 0.0;
    }
  }

  bool get canHaveEllipsis {
    if (paragraph.paragraphStyle.ellipsis == null) {
      return false;
    }

    final int? maxLines = paragraph.paragraphStyle.maxLines;
    return (maxLines == null) || (maxLines == lineNumber + 1);
  }

  /// Whether the [fragment] can fit in the line without exceeding [maxWidth].
  bool canAddFragment(MeasuredFragment fragment) {
    if (fragment.isSpaceOnly) {
      return true;
    }

    final double widthAfterAddingFragment =
        widthIncludingSpace + fragment.metrics.widthExcludingTrailingSpaces;
    return widthAfterAddingFragment <= maxWidth;
  }

  ui.TextDirection get _paragraphDirection =>
      paragraph.paragraphStyle.effectiveTextDirection;

  void addFragment(MeasuredFragment fragment) {
    fragment = _updateMetrics(fragment);

    if (fragment.type != LineBreakType.prohibited) {
      _lastBreakableFragment = _fragments.length;
    }

    _fragments.add(fragment);
  }

  void addPartOfFragment(MeasuredFragment fragment) {
    addFragment(fragment);
    _forceBreak(maxWidth, allowEmpty: false);
  }

  /// Updates the [LineBuilder]'s metrics after adding the new [fragment].
  MeasuredFragment _updateMetrics(MeasuredFragment fragment) {
    if (fragment.isSpaceOnly) {
      _spaceBoxCount++;
    } else {
      width = widthIncludingSpace + fragment.metrics.widthExcludingTrailingSpaces;
    }
    widthIncludingSpace += fragment.metrics.widthIncludingTrailingSpaces;

    if (fragment.isPlaceholder) {
      return _updateHeightForPlaceholder(fragment);
    }

    ascent = math.max(ascent, fragment.metrics.ascent);
    descent = math.max(descent, fragment.metrics.descent);
    return fragment;
  }

  MeasuredFragment _updateHeightForPlaceholder(MeasuredFragment fragment) {
    final PlaceholderSpan placeholder = fragment.span as PlaceholderSpan;

    final double ascent, descent;
    switch (placeholder.alignment) {
      case ui.PlaceholderAlignment.top:
        // The placeholder is aligned to the top of text, which means it has the
        // same `ascent` as the remaining text. We only need to extend the
        // `descent` enough to fit the placeholder.
        ascent = this.ascent;
        descent = placeholder.height - this.ascent;
        break;

      case ui.PlaceholderAlignment.bottom:
        // The opposite of `top`. The `descent` is the same, but we extend the
        // `ascent`.
        ascent = placeholder.height - this.descent;
        descent = this.descent;
        break;

      case ui.PlaceholderAlignment.middle:
        final double textMidPoint = height / 2;
        final double placeholderMidPoint = placeholder.height / 2;
        final double diff = placeholderMidPoint - textMidPoint;
        ascent = this.ascent + diff;
        descent = this.descent + diff;
        break;

      case ui.PlaceholderAlignment.aboveBaseline:
        ascent = placeholder.height;
        descent = 0.0;
        break;

      case ui.PlaceholderAlignment.belowBaseline:
        ascent = 0.0;
        descent = placeholder.height;
        break;

      case ui.PlaceholderAlignment.baseline:
        ascent = placeholder.baselineOffset;
        descent = placeholder.height - ascent;
        break;
    }

    this.ascent = math.max(this.ascent, ascent);
    this.descent = math.max(this.descent, descent);

    // Update the metrics of the fragment to reflect the calculated ascent and
    // descent.
    return fragment.copyWith(
      metrics: fragment.metrics.copyWith(
        ascent: ascent,
        descent: descent,
      ),
    );
  }

  void _recalculateMetrics() {
    width = 0;
    widthIncludingSpace = 0;
    ascent = 0;
    descent = 0;
    _spaceBoxCount = 0;
    _lastBreakableFragment = -1;

    for (int i = 0; i < _fragments.length; i++) {
      _fragments[i] = _updateMetrics(_fragments[i]);
      if (_fragments[i].type != LineBreakType.prohibited) {
        _lastBreakableFragment = i;
      }
    }
  }

  /// Force-breaks the line in order to fit in [maxWidth] while trying to extend
  /// to [nextBreak].
  ///
  /// This should only be called when there isn't enough width to extend to
  /// [nextBreak], and either of the following is true:
  ///
  /// 1. An ellipsis is being appended to this line, OR
  /// 2. The line doesn't have any line break opportunities and has to be
  ///    force-broken.
  void _forceBreak(double availableWidth, { required bool allowEmpty }) {
    MeasuredFragment? removedFragment;
    _fragmentsForNextLine = <MeasuredFragment>[];

    while (_fragments.isNotEmpty && widthIncludingSpace > availableWidth) {
      if (removedFragment != null) {
        _fragmentsForNextLine!.insert(0, removedFragment);
      }
      removedFragment = _fragments.removeLast();
      _recalculateMetrics();
    }

    if (removedFragment == null) {
      // No fragment has been removed.
      return;
    }

    final ParagraphSpan span = removedFragment.span;
    if (span is FlatTextSpan) {
      spanometer.currentSpan = span;
      final double availableWidthForFragment = availableWidth - widthIncludingSpace;
      final int breakingPoint = spanometer.forceBreak(
        removedFragment.start,
        removedFragment.end,
        availableWidth: availableWidthForFragment,
        allowEmpty: allowEmpty,
      );

      final List<LayoutFragment?> split = removedFragment.split(breakingPoint);
      if (split.first != null) {
        // _lastBreakableFragment = _fragments.length;
        addFragment(spanometer.measureFragment(split.first!));
      }
      if (split.last != null) {
        _fragmentsForNextLine!.insert(0, spanometer.measureFragment(split.last!));
      }

    } else {
      // There's no force-breaking, just remove the entire fragment.
      _fragmentsForNextLine!.insert(0, removedFragment);
    }
  }

  String? _ellipsis;

  void insertEllipsis() {
    assert(canHaveEllipsis);
    _ellipsis = paragraph.paragraphStyle.ellipsis!;
    final double ellipsisWidth = spanometer.measureText(_ellipsis!);
    final double availableWidth = maxWidth - ellipsisWidth;
    _forceBreak(availableWidth, allowEmpty: true);
  }

  /// Builds the [ParagraphLine] instance that represents this line.
  ParagraphLine build({String? ellipsis}) {
    if (_lastBreakableFragment < 0) {
      // TODO(mdebbar): Do we need to worry about this case?
      print('Building an unbreakable line!');
    }

    if (_fragmentsForNextLine == null) {
      _fragmentsForNextLine = _fragments.getRange(_lastBreakableFragment + 1, _fragments.length).toList();
      _fragments.removeRange(_lastBreakableFragment + 1, _fragments.length);
    }

    _processTrailingSpaces();

    final MeasuredFragment firstFragment = _fragments.first;
    final MeasuredFragment lastFragment = _fragments.last;

    return ParagraphLine(
      lineNumber: lineNumber,
      ellipsis: _ellipsis,
      startIndex: firstFragment.start,
      endIndex: lastFragment.end,
      trailingNewlines: 0, // TODO(mdebbar): lastFragment.trailingNewlines,
      hardBreak: lastFragment.isHardBreak,
      width: width,
      widthWithTrailingSpaces: widthIncludingSpace,
      left: alignOffset,
      height: height,
      baseline: accumulatedHeight + ascent,
      ascent: ascent,
      descent: descent,
      boxes: <RangeBox>[], // TODO(mdebbar): _boxes,
      spaceBoxCount: _spaceBoxCount,
      trailingSpaceBoxCount: _trailingSpaceBoxCount,
    );
  }

  int _trailingSpaceBoxCount = 0;

  void _processTrailingSpaces() {

    // TODO(mdebbar): If the fragments containing trailing spaces have the same
    // dir as the paragraph, then there's nothing to do.
    // Else, flip dir of the space-only trailing fragments, and potentially
    // split the fragment that contains text & trailing spaces.

    _trailingSpaceBoxCount = 0;
    for (int i = _boxes.length - 1; i >= 0; i--) {
      final RangeBox box = _boxes[i];
      final bool isSpaceBox = box is SpanBox && box.isSpaceOnly;
      if (!isSpaceBox) {
        // We traversed all trailing space boxes.
        break;
      }

      box._isTrailingSpace = true;
      _trailingSpaceBoxCount++;
    }
  }

  /// Creates a new [LineBuilder] to build the next line in the paragraph.
  LineBuilder nextLine() {
    return LineBuilder._(
      paragraph,
      spanometer,
      maxWidth: maxWidth,
      lineNumber: lineNumber + 1,
      accumulatedHeight: accumulatedHeight + height,
      fragments: _fragmentsForNextLine ?? <MeasuredFragment>[],
    );
  }
}

/// Responsible for taking measurements within spans of a paragraph.
///
/// Can't perform measurements across spans. To measure across spans, multiple
/// measurements have to be taken.
///
/// Before performing any measurement, the [currentSpan] has to be set. Once
/// it's set, the [Spanometer] updates the underlying [context] so that
/// subsequent measurements use the correct styles.
class Spanometer {
  Spanometer(this.paragraph, this.context);

  final CanvasParagraph paragraph;
  final DomCanvasRenderingContext2D context;

  static RulerHost _rulerHost = RulerHost();

  static Map<TextHeightStyle, TextHeightRuler> _rulers =
      <TextHeightStyle, TextHeightRuler>{};

  @visibleForTesting
  static Map<TextHeightStyle, TextHeightRuler> get rulers => _rulers;

  /// Clears the cache of rulers that are used for measuring text height and
  /// baseline metrics.
  static void clearRulersCache() {
    _rulers.forEach((TextHeightStyle style, TextHeightRuler ruler) {
      ruler.dispose();
    });
    _rulers.clear();
  }

  String _cssFontString = '';

  double? get letterSpacing => currentSpan.style.letterSpacing;

  TextHeightRuler? _currentRuler;
  FlatTextSpan? _currentSpan;

  FlatTextSpan get currentSpan => _currentSpan!;
  set currentSpan(FlatTextSpan? span) {
    if (span == _currentSpan) {
      return;
    }
    _currentSpan = span;

    // No need to update css font string when `span` is null.
    if (span == null) {
      _currentRuler = null;
      return;
    }

    // Update the height ruler.
    // If the ruler doesn't exist in the cache, create a new one and cache it.
    final TextHeightStyle heightStyle = span.style.heightStyle;
    TextHeightRuler? ruler = _rulers[heightStyle];
    if (ruler == null) {
      ruler = TextHeightRuler(heightStyle, _rulerHost);
      _rulers[heightStyle] = ruler;
    }
    _currentRuler = ruler;

    // Update the font string if it's different from the previous span.
    final String cssFontString = span.style.cssFontString;
    if (_cssFontString != cssFontString) {
      _cssFontString = cssFontString;
      context.font = cssFontString;
    }
  }

  /// Whether the spanometer is ready to take measurements.
  bool get isReady => _currentSpan != null;

  /// The distance from the top of the current span to the alphabetic baseline.
  double get ascent => _currentRuler!.alphabeticBaseline;

  /// The distance from the bottom of the current span to the alphabetic baseline.
  double get descent => height - ascent;

  /// The line height of the current span.
  double get height => _currentRuler!.height;

  double measureText(String text) {
    return measureSubstring(context, text, 0, text.length);
  }

  MeasuredFragment measureFragment(LayoutFragment fragment) {
    final FragmentMetrics metrics;

    if (fragment.isPlaceholder) {
      final PlaceholderSpan placeholder = fragment.span as PlaceholderSpan;
      // The ascent/descent values of the placeholder fragment will be finalized
      // later when the line is built.
      metrics = FragmentMetrics(
        ascent: placeholder.height,
        descent: 0,
        widthExcludingTrailingSpaces: placeholder.width,
        widthIncludingTrailingSpaces: placeholder.width,
      );
    } else {
      currentSpan = fragment.span as FlatTextSpan;
      final double widthExcludingTrailingSpaces = _measure(fragment.start, fragment.end - fragment.trailingSpaces);
      final double widthIncludingTrailingSpaces = _measure(fragment.start, fragment.end - fragment.trailingNewlines);
      metrics = FragmentMetrics(
        ascent: ascent,
        descent: descent,
        widthExcludingTrailingSpaces: widthExcludingTrailingSpaces,
        widthIncludingTrailingSpaces: widthIncludingTrailingSpaces,
      );
    }

    return MeasuredFragment(fragment, metrics);
  }

  /// In a continuous, unbreakable block of text from [start] to [end], finds
  /// the point where text should be broken to fit in the given [availableWidth].
  ///
  /// The [start] and [end] indices have to be within the same text span.
  ///
  /// When [allowEmpty] is true, the result is guaranteed to be at least one
  /// character after [start]. But if [allowEmpty] is false and there isn't
  /// enough [availableWidth] to fit the first character, then [start] is
  /// returned.
  ///
  /// See also:
  /// - [LineBuilder.forceBreak].
  int forceBreak(
    int start,
    int end, {
    required double availableWidth,
    required bool allowEmpty,
  }) {
    assert(_currentSpan != null);

    final FlatTextSpan span = currentSpan;

    // Make sure the range is within the current span.
    assert(start >= span.start && start <= span.end);
    assert(end >= span.start && end <= span.end);

    if (availableWidth <= 0.0) {
      return allowEmpty ? start : start + 1;
    }

    int low = start;
    int high = end;
    do {
      final int mid = (low + high) ~/ 2;
      final double width = _measure(start, mid);
      if (width < availableWidth) {
        low = mid;
      } else if (width > availableWidth) {
        high = mid;
      } else {
        low = high = mid;
      }
    } while (high - low > 1);

    if (low == start && !allowEmpty) {
      low++;
    }
    return low;
  }

  double _measure(int start, int end) {
    assert(_currentSpan != null);
    final FlatTextSpan span = currentSpan;

    // Make sure the range is within the current span.
    assert(start >= span.start && start <= span.end);
    assert(end >= span.start && end <= span.end);

    final String text = paragraph.toPlainText();
    return measureSubstring(
      context,
      text,
      start,
      end,
      letterSpacing: letterSpacing,
    );
  }
}

class MeasuredFragment implements LayoutFragment {
  const MeasuredFragment(this._layoutFragment, this.metrics);

  final LayoutFragment _layoutFragment;
  final FragmentMetrics metrics;

  @override
  int get start => _layoutFragment.start;
  @override
  int get end => _layoutFragment.end;
  @override
  LineBreakType get type => _layoutFragment.type;
  @override
  ui.TextDirection get textDirection => _layoutFragment.textDirection;
  @override
  ParagraphSpan get span => _layoutFragment.span;

  @override
  int get trailingNewlines => _layoutFragment.trailingNewlines;
  @override
  int get trailingSpaces => _layoutFragment.trailingSpaces;

  @override
  int get length => _layoutFragment.length;
  @override
  bool get isSpaceOnly => _layoutFragment.isSpaceOnly;
  @override
  bool get isHardBreak => _layoutFragment.isHardBreak;
  @override
  bool get isPlaceholder => _layoutFragment.isPlaceholder;

  MeasuredFragment copyWith({
    LayoutFragment? layoutFragment,
    FragmentMetrics? metrics,
  }) {
    return MeasuredFragment(
      layoutFragment ?? _layoutFragment,
      metrics ?? this.metrics,
    );
  }

  List<LayoutFragment?> split(int index) {
    assert(start <= index);
    assert(index <= end);

    if (start == index) {
      return <LayoutFragment?>[null, _layoutFragment];
    }

    if (end == index) {
      return <LayoutFragment?>[_layoutFragment, null];
    }

    final int secondLength = end - index;
    return <LayoutFragment>[
      LayoutFragment(
        start,
        index,
        LineBreakType.prohibited,
        textDirection,
        span,
        trailingNewlines: math.max(0, trailingNewlines - secondLength),
        trailingSpaces: math.max(0, trailingSpaces - secondLength),
      ),
      LayoutFragment(
        index,
        end,
        type,
        textDirection,
        span,
        trailingNewlines: math.min(trailingNewlines, secondLength),
        trailingSpaces: math.min(trailingSpaces, secondLength),
      ),
    ];
  }
}

/// Immutable data class that holds measurement results for a fragment of text.
class FragmentMetrics {
  const FragmentMetrics({
    required this.ascent,
    required this.descent,
    required this.widthExcludingTrailingSpaces,
    required this.widthIncludingTrailingSpaces,
  });

  static const FragmentMetrics zero = FragmentMetrics(
    ascent: 0,
    descent: 0,
    widthExcludingTrailingSpaces: 0,
    widthIncludingTrailingSpaces: 0,
  );

  /// The rise from the baseline as calculated from the font and style for this text.
  final double ascent;

  /// The drop from the baseline as calculated from the font and style for this text.
  final double descent;

  /// The width of the measured text, not including trailing spaces.
  final double widthExcludingTrailingSpaces;

  /// The width of the measured text, including any trailing spaces.
  final double widthIncludingTrailingSpaces;

  /// The width of trailing spaces in the measured text.
  double get widthOfTrailingSpaces =>
      widthIncludingTrailingSpaces - widthExcludingTrailingSpaces;

  /// The total height as calculated from the font and style for this text.
  double get height => ascent + descent;

  FragmentMetrics copyWith({
    double? ascent,
    double? descent,
    double? widthExcludingTrailingSpaces,
    double? widthIncludingTrailingSpaces,
  }) {
    return FragmentMetrics(
      ascent: ascent ?? this.ascent,
      descent: descent ?? this.descent,
      widthExcludingTrailingSpaces: widthExcludingTrailingSpaces ?? this.widthExcludingTrailingSpaces,
      widthIncludingTrailingSpaces: widthIncludingTrailingSpaces ?? this.widthIncludingTrailingSpaces,
    );
  }
}
