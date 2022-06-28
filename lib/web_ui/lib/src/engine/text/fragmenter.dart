// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;

import 'package:ui/ui.dart' as ui;

import 'canvas_paragraph.dart';
import 'line_breaker.dart';
import 'text_direction.dart';

abstract class TextFragmenter {
  const TextFragmenter(this.paragraph);

  final CanvasParagraph paragraph;

  List<TextFragment> fragment();
}

abstract class TextFragment {
  const TextFragment(this.start, this.end);

  final int start;
  final int end;
}

class LayoutFragmenter extends TextFragmenter {
  const LayoutFragmenter(super.paragraph);

  @override
  List<LayoutFragment> fragment() {
    final List<LayoutFragment> fragments = <LayoutFragment>[];

    int fragmentStart = 0;

    final Iterator<LineBreakFragment> lineBreakFragments =
        LineBreakFragmenter(paragraph).fragment().iterator..moveNext();
    final Iterator<BidiFragment> bidiFragments =
        BidiFragmenter(paragraph).fragment().iterator..moveNext();
    final Iterator<ParagraphSpan> spans = paragraph.spans.iterator..moveNext();

    LineBreakFragment currentLineBreakFragment = lineBreakFragments.current;
    BidiFragment currentBidiFragment = bidiFragments.current;
    ParagraphSpan currentSpan = spans.current;

    while (true) {
      final int fragmentEnd = math.min(
        currentLineBreakFragment.end,
        math.min(
          currentBidiFragment.end,
          currentSpan.end,
        ),
      );

      final int distanceFromLineBreak = currentLineBreakFragment.end - fragmentEnd;

      final LineBreakType lineBreakType = distanceFromLineBreak == 0
          ? currentLineBreakFragment.type
          : LineBreakType.prohibited;

      final int trailingNewlines = currentLineBreakFragment.trailingNewlines - distanceFromLineBreak;
      final int trailingSpaces = currentLineBreakFragment.trailingSpaces - distanceFromLineBreak;

      fragments.add(LayoutFragment(
        fragmentStart,
        fragmentEnd,
        lineBreakType,
        currentBidiFragment.textDirection,
        currentSpan,
        trailingNewlines: math.max(0, trailingNewlines),
        trailingSpaces: math.max(0, trailingSpaces),
      ));

      fragmentStart = fragmentEnd;

      bool moved = false;
      if (currentLineBreakFragment.end == fragmentEnd) {
        if (lineBreakFragments.moveNext()) {
          moved = true;
          currentLineBreakFragment = lineBreakFragments.current;
        }
      }
      if (currentBidiFragment.end == fragmentEnd) {
        if (bidiFragments.moveNext()) {
          moved = true;
          currentBidiFragment = bidiFragments.current;
        }
      }
      if (currentSpan.end == fragmentEnd) {
        if (spans.moveNext()) {
          moved = true;
          currentSpan = spans.current;
        }
      }

      // Once we reached the end of all fragments, exit the loop.
      if (!moved) {
        break;
      }
    }

    return fragments;
  }
}

class LayoutFragment extends TextFragment implements LineBreakFragment, BidiFragment {
  const LayoutFragment(
    super.start,
    super.end,
    this.type,
    this.textDirection,
    this.span, {
    required this.trailingNewlines,
    required this.trailingSpaces,
  });

  @override
  final LineBreakType type;

  @override
  final ui.TextDirection textDirection;

  final ParagraphSpan span;

  @override
  final int trailingNewlines;

  @override
  final int trailingSpaces;

  int get length => end - start;
  bool get isSpaceOnly => length == trailingSpaces;
  bool get isPlaceholder => span is PlaceholderSpan;
  bool get isHardBreak => type == LineBreakType.mandatory || type == LineBreakType.endOfText;
}
