// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This library is self-contained and has no imports.  This allows it to be
// imported from outside the engine by `lib/web_ui/dev/roll_fallback_fonts.dart`.

class NotoFont {
  NotoFont(this.name, this.url, this._packedRanges);

  final String name;
  final String url;
  final String _packedRanges;
  // A sorted list of Unicode ranges.
  late final List<CodePointRange> _ranges = _unpackFontRanges(_packedRanges);

  List<CodePointRange> computeUnicodeRanges() => _ranges;

  // Returns `true` if this font has a glyph for the given [codeunit].
  bool contains(int codeUnit) {
    // Binary search through the unicode ranges to see if there
    // is a range that contains the codeunit.
    int min = 0;
    int max = _ranges.length - 1;
    while (min <= max) {
      final int mid = (min + max) ~/ 2;
      final CodePointRange range = _ranges[mid];
      if (range.start > codeUnit) {
        max = mid - 1;
      } else {
        // range.start <= codeUnit
        if (range.end >= codeUnit) {
          return true;
        }
        min = mid + 1;
      }
    }
    return false;
  }
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


const _kPrefixDigit0 = 48; // '0'
const _kPrefixRadix = 10;
const _kSkipDigit0 = 65;  // 'A'
const _kSkipRadix = 26;
const _kSizeDigit0 = 65 + 32; // 'a'
const _kSizeRadix = 26;

List<CodePointRange> _unpackFontRanges(String packedRange) {
  final List<CodePointRange> ranges = <CodePointRange>[];
  
  int lowestPossibleStart = 0;
  int prefix = 0;
  int size = 1;
  
  for (int i = 0; i < packedRange.length; i++) {
    final code = packedRange.codeUnitAt(i);
    if (_kSkipDigit0 <= code && code < _kSkipDigit0 + _kSkipRadix) {
      final int skip = prefix * _kSkipRadix + (code - _kSkipDigit0);
      final int rangeStart = lowestPossibleStart + skip;
      final int rangeEnd = rangeStart + size - 1;
      ranges.add(CodePointRange(rangeStart, rangeEnd));
      lowestPossibleStart = rangeEnd + 2;
      prefix = 0;
      size = 1;
    } else if (_kSizeDigit0 <= code && code < _kSizeDigit0 + _kSizeRadix) {
      size = prefix * _kSizeRadix + (code - _kSizeDigit0) + 2;
      prefix = 0;
    } else if (_kPrefixDigit0 <= code && code < _kPrefixDigit0 + _kPrefixRadix) {
      prefix = prefix * _kPrefixRadix + (code - _kPrefixDigit0);
    } else {
      throw StateError('Unreachable');
    }
  }

  return ranges;
}


/// Returns a String for the packed font ranges that can be unpacked by
/// [_unpackFontRanges].
///
/// A font contains characters for a subset of the possible unicode code points.
/// The set of code points is represented as a list of `[start, end]` ranges, in
/// order of increasing start code-point.  Ranges are non-overlapping and
/// non-abutting, otherwise they would have been merged into a single range, so
/// there is always a gap between two ranges.  This list of ranges is encoded as
/// a string of ASCII characters, avoiding characters like control codes and
/// quotes that would need escaping in a String literal when compiled to
/// JavaScript.
///
/// Various techniques are used to make the encoding more compact.
///
/// The ranges are represented as a sequence of (skip, size) pairs.  _skip_ is a
/// count of the code points in the gap between ranges. _size_ is the number of
/// code points in the range.  This 'delta' encoding is efficient because large
/// numbers rarely appear, and it compresses well since similar patterns of
/// ranges now encode to the same sequence of skip/size deltas.
///
/// Since there is always at least one code point in the gap, _skip_ counts the
/// _extra_ code points, so a _skip_ count of 0 means a gap of one code point
/// and a _skip_ count of 1 means a gap of two code points etc.
///
/// Many gaps and ranges are small, so separator characters would be a
/// significant proportion of the encoding. Instead values are encoded with two
/// kinds of digits: prefix digits and terminating digits. Lets say we use
/// decimal digits `0`..`9` for prefix digits and `A`..`Z` as terminating
/// digits. This is a multi-radix encoding:
///
///    M = ('M' - 'A') = 12
///    38M = (3 * 10 + 8) * 26 + 12 = 38 * 26 + 12 = 1000
//
/// With this encoding, the minimal _(skip, size)_ pair _(1,1)_ becomes two
/// characters `AA`.
//
/// Singleton ranges are very common - about 40% of the ranges are singletons.
/// It is beneficial to simply omit the _size_ in this case. To detect a missing
/// _size_ (1) sizes are encoded with a different kind of terminating digit
/// (say, `a`..`z`), and (2) the skip count comes after the size. Because the
/// size is omitted when it is one, a bias of 2 is used for an encoded size.
///
/// Example encodings:
///
/// | encoding | skip | size |
/// | :---     | ---: | ---: |
/// | A        | 1    | 1    |
/// | B        | 2    | 1    |
/// | 38M      | 1000 | 1    |
/// | aA       | 1    | 2    |
/// | bB       | 2    | 3    |
/// | 1a1A     | 26   | 27   |
/// | 38a38M   | 1000 | 1001 |
///
String packFontRanges(List<CodePointRange> ranges) {
  final StringBuffer sb = StringBuffer();

  void encode(int value, int radix, int firstDigitCode) {
    final int prefix = value ~/ radix;
    assert(_kPrefixDigit0 == '0'.codeUnitAt(0) && _kPrefixRadix == 10);
    if (prefix != 0) sb.write(prefix);
    sb.writeCharCode(firstDigitCode + value.remainder(radix));
  }

  int lowestPossibleStart = 0;
  for (final CodePointRange range in ranges) {
    final int start = range.start;
    final int end = range.end;

    // Encode range as `<size><skip>` where _skip_ is how many code points to
    // skip to get to the range start, and _size_ is how many code points are in
    // the range. _size_ is omitted for singleton ranges.
    final int skip = start - lowestPossibleStart;
    final int size = end - start + 1;
    if (size >= 2) {
      // Omit the size for singleton ranges and bias the size of longer ranges
      // by 2.
      encode(size - 2, _kSizeRadix, _kSizeDigit0);
    }
    encode(skip, _kSkipRadix, _kSkipDigit0);

    // Next range can't start until after at least a singleton gap. This is
    // where we bias the skip by 1 but allow the first range to start at code
    // point 0.
    lowestPossibleStart = end + 2;
  }
  return sb.toString();
}
