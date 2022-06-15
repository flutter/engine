// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'canvas_paragraph.dart';

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
