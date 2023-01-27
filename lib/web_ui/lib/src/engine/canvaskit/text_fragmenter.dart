// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import '../text/line_breaker.dart';

abstract class CkTextFragmenter {
  const CkTextFragmenter(this.text);

  /// The text to be fragmented.
  final String text;

  /// Performs the fragmenting of [text] and returns a [Uint32List].
  Uint32List fragment();
}

class CkWordFragmenter extends CkTextFragmenter {
  CkWordFragmenter(super.text);

  @override
  Uint32List fragment() {
    // TODO(mdebbar): Use Intl.Segmenter to get actual word boundaries.
    return Uint32List.fromList(<int> [0, text.length]);
  }
}

class CkGraphemeBreakFragmenter extends CkTextFragmenter {
  CkGraphemeBreakFragmenter(super.text);

  @override
  Uint32List fragment() {
    // TODO(mdebbar): Use Intl.Segmenter to get actual grapheme boundaries.
    return Uint32List.fromList(List<int>.generate(text.length + 1, (int i) => i));
  }
}

// These are the soft/hard line break values expected by SkParagraph.
const int _kSoftLineBreak = 0;
const int _kHardLineBreak = 1;

class CkLineBreakFragmenter extends CkTextFragmenter {
  CkLineBreakFragmenter(super.text) : _fragmenter = LineBreakFragmenter(text);

  final LineBreakFragmenter _fragmenter;

  @override
  Uint32List fragment() {
    final List<LineBreakFragment> fragments = _fragmenter.fragment();

    final int size = (fragments.length + 1) * 2;
    final Uint32List typedArray = Uint32List(size);

    typedArray[0] = 0; // start index
    typedArray[1] = _kSoftLineBreak; // break type

    for (int i = 0; i < fragments.length; i++) {
      final LineBreakFragment fragment = fragments[i];
      final int uint32Index = 2 + i * 2;
      typedArray[uint32Index] = fragment.end;
      typedArray[uint32Index + 1] = fragment.type == LineBreakType.mandatory
          ? _kHardLineBreak
          : _kSoftLineBreak;
    }

    return typedArray;
  }
}
