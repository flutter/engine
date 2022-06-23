// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math' as math;

import 'package:ui/ui.dart' as ui;

import 'canvas_paragraph.dart';
import 'line_breaker.dart';
import 'paragraph.dart';
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

    // TODO(mdebbar): What if the paragraphs has 0 spans???

    int fragmentStart = 0;

    final Iterator<LineBreakFragment> lineBreakFragments =
        LineBreakFragmenter(paragraph).fragment().iterator..moveNext();
    final Iterator<BidiFragment> bidiFragments =
        BidiFragmenter(paragraph).fragment().iterator..moveNext();
    final Iterator<FlatTextSpan> spans =
        paragraph.spans.whereType<FlatTextSpan>().iterator..moveNext();

    LineBreakFragment currentLineBreakFragment = lineBreakFragments.current;
    BidiFragment currentBidiFragment = bidiFragments.current;
    FlatTextSpan currentSpan = spans.current;

    while (true) {
      final int minEnd = math.min(
        currentLineBreakFragment.end,
        math.min(
          currentBidiFragment.end,
          currentSpan.end,
        ),
      );

      final LineBreakType lineBreakType = currentLineBreakFragment.end == minEnd
          ? currentLineBreakFragment.endBreak.type
          : LineBreakType.prohibited;

      fragments.add(LayoutFragment(
        fragmentStart,
        minEnd,
        lineBreakType,
        currentBidiFragment.textDirection,
        currentSpan.style,
      ));

      fragmentStart = minEnd;

      bool moved = false;
      if (currentLineBreakFragment.end == minEnd) {
        if (lineBreakFragments.moveNext()) {
          moved = true;
          currentLineBreakFragment = lineBreakFragments.current;
        }
      }
      if (currentBidiFragment.end == minEnd) {
        if (bidiFragments.moveNext()) {
          moved = true;
          currentBidiFragment = bidiFragments.current;
        }
      }
      if (currentSpan.end == minEnd) {
        if (spans.moveNext()) {
          moved = true;
          currentSpan = spans.current;
        }
      }

      if (!moved) {
        break;
      }

      // 1. Find the min end among the three current fragments.
      // 2. Create (and add) a LayoutFragment using the above min as an end.
      //    2.a. If the min was a line break fragment, then use its break type.
      //    2.b. Else, use `prohibited`.
      // 3. Move forward the min fragment.

      // Think about end-of-text:
      // - All last fragments should have the same end.
      // - Line break fragment may have two fragments at the end (mandatory + endOfText).
      // - Maybe change the above algorithm to move forward all the fragments that share the min end?
      // - STOP! (i.e. `break;`)
    }

    // TODO(mdebbar): IMPLEMENT!

    return fragments;
  }
}

class LayoutFragment extends TextFragment {
  const LayoutFragment(super.start, super.end, this.type, this.textDirection, this.style);

  final LineBreakType type;
  final ui.TextDirection textDirection;
  final EngineTextStyle style;
}
