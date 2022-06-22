// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import '../html/paragraph/helper.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  group('$BidiFragmenter', () {
    test('basic cases', () {
      expect(split('Lorem 12 $rtlWord1   ipsum34', ltr), <_Bidi>[
        _Bidi('Lorem 12 ', ltr),
        _Bidi('$rtlWord1   ', rtl),
        _Bidi('ipsum34', ltr),
      ]);
      expect(split('Lorem 12 $rtlWord1   ipsum34', rtl), <_Bidi>[
        _Bidi('Lorem 12 ', ltr),
        _Bidi('$rtlWord1   ', rtl),
        _Bidi('ipsum34', ltr),
      ]);
    });

    test('spaces', () {
      expect(split('    ', ltr), <_Bidi>[
        _Bidi('    ', ltr),
      ]);
      expect(split('    ', rtl), <_Bidi>[
        _Bidi('    ', rtl),
      ]);
    });

    test('symbols', () {
      expect(split('Calculate 2.2 + 4.5 and write the result', ltr), <_Bidi>[
        _Bidi('Calculate 2.2 + 4.5 and write the result', ltr),
      ]);
      expect(split('Calculate 2.2 + 4.5 and write the result', rtl), <_Bidi>[
        _Bidi('Calculate 2.2 + 4.5 and write the result', ltr),
      ]);

      expect(split('Calculate $rtlWord1 2.2 + 4.5 and write the result', ltr), <_Bidi>[
        _Bidi('Calculate ', ltr),
        _Bidi('$rtlWord1 2.2 + 4.5 ', rtl),
        _Bidi('and write the result', ltr),
      ]);
      expect(split('Calculate $rtlWord1 2.2 + 4.5 and write the result', rtl), <_Bidi>[
        _Bidi('Calculate ', ltr),
        _Bidi('$rtlWord1 2.2 + 4.5 ', rtl),
        _Bidi('and write the result', ltr),
      ]);

      expect(split('$rtlWord1 2.2 + 4.5 foo', ltr), <_Bidi>[
        _Bidi('$rtlWord1 2.2 + 4.5 ', rtl),
        _Bidi('foo', ltr),
      ]);
      expect(split('$rtlWord1 2.2 + 4.5 foo', rtl), <_Bidi>[
        _Bidi('$rtlWord1 2.2 + 4.5 ', rtl),
        _Bidi('foo', ltr),
      ]);

      expect(split('12 + 24 = 36', ltr), <_Bidi>[
        _Bidi('12 + 24 = 36', ltr),
      ]);
      expect(split('12 + 24 = 36', rtl), <_Bidi>[
        _Bidi('12 + 24 = 36', rtl),
      ]);
    });

    test('handles new lines', () {
      expect(split('Lorem\n12\nipsum  \n', ltr), <_Bidi>[
        _Bidi('Lorem\n12\nipsum  \n', ltr),
      ]);
      expect(split('Lorem\n12\nipsum  \n', rtl), <_Bidi>[
        _Bidi('Lorem\n12\nipsum  \n', ltr),
      ]);

      expect(split('$rtlWord1\n  $rtlWord2 \n', ltr), <_Bidi>[
        _Bidi('$rtlWord1\n  $rtlWord2 \n', rtl),
      ]);
      expect(split('$rtlWord1\n  $rtlWord2 \n', rtl), <_Bidi>[
        _Bidi('$rtlWord1\n  $rtlWord2 \n', rtl),
      ]);
    });
  });
}

/// Holds information about how a bidi region was split from a string.
class _Bidi {
  _Bidi(this.text, this.textDirection);

  final String text;
  final TextDirection textDirection;

  factory _Bidi.fromBidiFragment(String text, BidiFragment bidiFragment) {
    return _Bidi(
      text.substring(bidiFragment.start, bidiFragment.end),
      bidiFragment.textDirection,
    );
  }

  @override
  int get hashCode => Object.hash(text, textDirection);

  @override
  bool operator ==(Object other) {
    return other is _Bidi &&
        other.text == text &&
        other.textDirection == textDirection;
  }

  @override
  String toString() {
    return '"$text" ($textDirection)';
  }
}

List<_Bidi> split(String text, TextDirection textDirection) {
  return <_Bidi>[
    for (final BidiFragment bidiFragment in computeBidiFragments(text, textDirection))
      _Bidi.fromBidiFragment(text, bidiFragment)
  ];
}

List<BidiFragment> computeBidiFragments(String text, TextDirection textDirection) {
  final CanvasParagraph paragraph = plain(EngineParagraphStyle(textDirection: textDirection), text);
  final BidiFragmenter fragmenter = BidiFragmenter(paragraph);
  return fragmenter.fragment();
}
