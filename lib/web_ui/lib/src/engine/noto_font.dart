// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'text/unicode_range.dart';

class NotoFont {
  NotoFont(this.name, this.url, {this.enabled = true});

  final String name;
  final String url;
  final bool enabled;

  final int index = _index++;
  static int _index = 0;

  /// During fallback font selection this is the number of missing code points
  /// that are covered by (i.e. in) this font.
  int coverCount = 0;

  /// During fallback font selection this is a list of [FallbackFontBlock]s that
  /// contribute to this font's cover count.
  final List<FallbackFontBlock> coverBlocks = [];
}

// FallbackFontCodePointsPartitionComponent.
class FallbackFontBlock {
  FallbackFontBlock(this.fonts);
  final List<NotoFont> fonts;

  /// During fallback font selection this is the number of missing code points
  /// that are covered by (i.e. in) the intersection of all [fonts].
  int coverCount = 0;
}



class CodePointRange {
  const CodePointRange(this.start, this.end);

  final int start;
  final int end;

  bool contains(int codeUnit) {
    return start <= codeUnit && codeUnit <= end;
  }

  @override
  bool operator ==(Object other) {
    if (other is! CodePointRange) {
      return false;
    }
    final CodePointRange range = other;
    return range.start == start && range.end == end;
  }

  @override
  int get hashCode => Object.hash(start, end);

  @override
  String toString() => '[$start, $end]';
}
