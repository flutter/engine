// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('Should be able to build and layout a paragraph', () {
    final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('Hello');
    final Paragraph paragraph = builder.build();
    expect(paragraph, isNotNull);

    paragraph.layout(const ParagraphConstraints(width: 800.0));
    expect(paragraph.width, isNonZero);
    expect(paragraph.height, isNonZero);
  });

  test('PushStyle should not segfault after build()', () {
    final ParagraphBuilder paragraphBuilder =
        ParagraphBuilder(ParagraphStyle());
    paragraphBuilder.build();
    paragraphBuilder.pushStyle(TextStyle());
  });

  test('GetRectsForRange smoke test', () {
    final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('Hello');
    final Paragraph paragraph = builder.build();
    expect(paragraph, isNotNull);

    paragraph.layout(const ParagraphConstraints(width: 800.0));
    expect(paragraph.width, isNonZero);
    expect(paragraph.height, isNonZero);

    final List<TextBox> boxes = paragraph.getBoxesForRange(0, 3);
    expect(boxes.length, 1);
    expect(boxes.first, const TextBox.fromLTRBD(0, 0, 42, 14, TextDirection.ltr));
  });

  test('LineMetrics smoke test', () {
    final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('Hello');
    final Paragraph paragraph = builder.build();
    expect(paragraph, isNotNull);

    paragraph.layout(const ParagraphConstraints(width: 800.0));
    expect(paragraph.width, isNonZero);
    expect(paragraph.height, isNonZero);

    final List<LineMetrics> metrics = paragraph.computeLineMetrics();
    expect(metrics.length, 1);
    // The actual values here will vary by platform slightly.
    const double epsillon = 0.00001;
    expect(metrics.first.hardBreak, true);
    expect(metrics.first.ascent, closeTo(11.200042724609375, epsillon));
    expect(metrics.first.descent, closeTo(2.799957275390625, epsillon));
    expect(metrics.first.unscaledAscent, closeTo(11.200042724609375, epsillon));
    expect(metrics.first.height, 14.0);
    expect(metrics.first.width, 70.0);
    expect(metrics.first.left, 0.0);
    expect(metrics.first.baseline, closeTo(11.200042724609375, epsillon));
    expect(metrics.first.lineNumber, 0);
  });
}
