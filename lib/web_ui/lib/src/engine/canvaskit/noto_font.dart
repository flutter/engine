// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class NotoFont {
  final String name;
  final String url;
  final List<CodeunitRange> unicodeRanges;

  const NotoFont(this.name, this.url, this.unicodeRanges);

  // Returns `true` if this font has a glyph for the given [codeunit].
  bool contains(int codeunit) {
    // The list of codeunit ranges is sorted.
    for (final CodeunitRange range in unicodeRanges) {
      if (range.start > codeunit) {
        return false;
      } else {
        // range.start <= codeunit
        if (range.end >= codeunit) {
	  return true;
	}
      }
    }
    return false;
  }
}

class CodeunitRange {
  final int start;
  final int end;

  const CodeunitRange(this.start, this.end);

  bool contains(int codeUnit) {
    return start <= codeUnit && codeUnit <= end;
  }

  @override
  bool operator ==(dynamic other) {
    if (other is! CodeunitRange) {
      return false;
    }
    final CodeunitRange range = other;
    return range.start == start && range.end == end;
  }

  @override
  int get hashCode => Object.hash(start, end);

  @override
  String toString() => '[$start, $end]';
}
