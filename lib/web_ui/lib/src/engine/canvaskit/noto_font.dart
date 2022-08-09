// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

class NotoFont {
  const NotoFont(this.name, this.url, List<int> unicodeRanges) {
    List<int> starts = <int>[];
    List<int> ends = <int>[];
    for (int i = 0; i < unicodeRanges.length; i += 2) {
      starts.add(unicodeRanges[i]);
      ends.add(unicodeRanges[i+1]);
    }
    this._rangeStarts = starts;
    this._rangeEnds = ends;
  }

  final String name;
  final String url;
  // A sorted list of Unicode range start points.
  final List<int> _rangeStarts;

  // A sorted list of Unicode range end points.
  final List<int> _rangeEnds;


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
  const CodeunitRange(this.start, this.end);

  final int start;
  final int end;


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
