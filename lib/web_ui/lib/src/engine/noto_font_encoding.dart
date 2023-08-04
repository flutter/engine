// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const kMaxCodePoint = 0x10ffff;

const kPrefixDigit0 = 48;
const kPrefixRadix = 10;

const kFontIndexDigit0 = 65 + 32; // 'a'..'z'
const kFontIndexRadix = 26;

const kFontSetDefineDigit0 = 65; // 'A'..'Z'
const kFontSetDefineRadix = 26;

const kFontSetDefineAndReset = 33; // '!'

const kRangeSizeDigit0 = 65 + 32; // 'a'..'z'
const kRangeSizeRadix = 26;

const kRangeSetDigit0 = 65; // 'A'..'Z'
const kRangeSetRadix = 26;


class _DigitRange {
  final int first;
  final int last;
  final int radix;
  const _DigitRange(this.first, this.radix) : last = first + radix - 1;

  bool matches(int code) => first <= code && code <= last;

  int combine(int value, int code) => value * radix + (code - first);
}

const kPrefixDigit = _DigitRange(48, 10);
