// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import '../dom.dart';
import '../text/line_breaker.dart';

abstract class CkTextFragmenter {
  const CkTextFragmenter(this.text);

  /// The text to be fragmented.
  final String text;

  /// Performs the fragmenting of [text] and returns a [Uint32List].
  Uint32List fragment();
}

/// The granularity at which to segment text.
///
/// To find all supported granularities, see:
/// - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Segmenter/Segmenter
enum IntlSegmenterGranularity {
  grapheme,
  word,
}

class CkIntlFragmenter extends CkTextFragmenter {
  CkIntlFragmenter(super.text, this.granularity)
      : assert(domIntl.Segmenter != null);

  final IntlSegmenterGranularity granularity;

  String get _granularityString {
    switch (granularity) {
      case IntlSegmenterGranularity.grapheme:
        return 'grapheme';
      case IntlSegmenterGranularity.word:
        return 'word';
    }
  }

  @override
  Uint32List fragment() {
    final DomIteratorWrapper<DomSegment> iterator =
        createIntlSegmenter(granularity: _granularityString)
            .segment(text)
            .iterator();

    final List<int> breaks = <int>[];
    while (iterator.moveNext()) {
      breaks.add(iterator.current.index);
    }
    breaks.add(text.length);

    return Uint32List.fromList(breaks);
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
