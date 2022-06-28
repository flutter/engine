// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import 'fragmenter.dart';
import 'unicode_range.dart';

class BidiFragmenter extends TextFragmenter {
  const BidiFragmenter(super.paragraph);

  @override
  List<BidiFragment> fragment() {
    return _computeBidiFragments(paragraph.toPlainText(), paragraph.paragraphStyle.effectiveTextDirection);
  }
}

class BidiFragment extends TextFragment {
  const BidiFragment(super.start, super.end, this.textDirection);

  final ui.TextDirection textDirection;

  @override
  int get hashCode => Object.hash(start, end, textDirection);

  @override
  bool operator ==(Object other) {
    return other is BidiFragment &&
        other.start == start &&
        other.end == end &&
        other.textDirection == textDirection;
  }

  @override
  String toString() {
    return 'BidiFragment($start, $end, $textDirection)';
  }
}

// This data was taken from the source code of the Closure library:
//
// - https://github.com/google/closure-library/blob/9d24a6c1809a671c2e54c328897ebeae15a6d172/closure/goog/i18n/bidi.js#L203-L234
final UnicodePropertyLookup<ui.TextDirection?> _textDirectionLookup = UnicodePropertyLookup<ui.TextDirection?>(
  <UnicodeRange<ui.TextDirection>>[
    // LTR
    const UnicodeRange<ui.TextDirection>(kChar_A, kChar_Z, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(kChar_a, kChar_z, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0x00C0, 0x00D6, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0x00D8, 0x00F6, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0x00F8, 0x02B8, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0x0300, 0x0590, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0x0591, 0x06EF, ui.TextDirection.rtl),
    const UnicodeRange<ui.TextDirection>(0x06FA, 0x08FF, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0x0900, 0x1FFF, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0x200E, 0x200E, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0x200F, 0x200F, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0x2C00, 0xD801, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0xD802, 0xD803, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0xD804, 0xD839, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0xD83A, 0xD83B, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0xD83C, 0xDBFF, ui.TextDirection.ltr),
    const UnicodeRange<ui.TextDirection>(0xF900, 0xFB1C, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0xFB1D, 0xFDFF, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0xFE00, 0xFE6F, ui.TextDirection.ltr),
    // RTL
    const UnicodeRange<ui.TextDirection>(0xFE70, 0xFEFC, ui.TextDirection.rtl),
    // LTR
    const UnicodeRange<ui.TextDirection>(0xFEFD, 0xFFFF, ui.TextDirection.ltr),
  ],
  null,
);

List<BidiFragment> _computeBidiFragments(String text, ui.TextDirection baseDirection) {
  final List<BidiFragment> fragments = <BidiFragment>[];

  int fragmentStart = 0;
  ui.TextDirection fragmentDirection = _textDirectionLookup.find(text, 0) ?? baseDirection;

  for (int i = 1; i < text.length; i++) {
    final ui.TextDirection charDirection = _textDirectionLookup.find(text, i) ?? fragmentDirection;
    if (charDirection != fragmentDirection) {
      // We've reached the end of a text direction fragment.
      fragments.add(BidiFragment(fragmentStart, i, fragmentDirection));
      fragmentStart = i;
      fragmentDirection = charDirection;
    }
  }

  fragments.add(BidiFragment(fragmentStart, text.length, fragmentDirection));
  return fragments;
}
