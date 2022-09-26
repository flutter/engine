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
      expect(split('Lorem 11 $rtlWord1  22 ipsum'), <_Bidi>[
        _Bidi('Lorem', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('11', ltr, previous),
        _Bidi(' ', null, sandwich),
        _Bidi(rtlWord1, rtl, own),
        _Bidi('  ', null, sandwich),
        _Bidi('22', ltr, previous),
        _Bidi(' ', null, sandwich),
        _Bidi('ipsum', ltr, own),
      ]);
    });

    test('text and digits', () {
      expect(split('Lorem11 ${rtlWord1}22 33ipsum44dolor ${rtlWord2}55$rtlWord1'), <_Bidi>[
        _Bidi('Lorem11', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi(rtlWord1, rtl, own),
        _Bidi('22', ltr, previous),
        _Bidi(' ', null, sandwich),
        _Bidi('33ipsum44dolor', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi(rtlWord2, rtl, own),
        _Bidi('55', ltr, previous),
        _Bidi(rtlWord1, rtl, own),
      ]);
    });

    test('spaces', () {
      expect(split('    '), <_Bidi>[
        _Bidi('    ', null, sandwich),
      ]);
    });

    test('symbols', () {
      expect(split('Calculate 2.2 + 4.5 and write the result'), <_Bidi>[
        _Bidi('Calculate', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('2', ltr, previous),
        _Bidi('.', null, sandwich),
        _Bidi('2', ltr, previous),
        _Bidi(' + ', null, sandwich),
        _Bidi('4', ltr, previous),
        _Bidi('.', null, sandwich),
        _Bidi('5', ltr, previous),
        _Bidi(' ', null, sandwich),
        _Bidi('and', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('write', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('the', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('result', ltr, own),
      ]);

      expect(split('Calculate $rtlWord1 2.2 + 4.5 and write the result'), <_Bidi>[
        _Bidi('Calculate', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi(rtlWord1, rtl, own),
        _Bidi(' ', null, sandwich),
        _Bidi('2', ltr, previous),
        _Bidi('.', null, sandwich),
        _Bidi('2', ltr, previous),
        _Bidi(' + ', null, sandwich),
        _Bidi('4', ltr, previous),
        _Bidi('.', null, sandwich),
        _Bidi('5', ltr, previous),
        _Bidi(' ', null, sandwich),
        _Bidi('and', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('write', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('the', ltr, own),
        _Bidi(' ', null, sandwich),
        _Bidi('result', ltr, own),
      ]);

      expect(split('12 + 24 = 36'), <_Bidi>[
        _Bidi('12', ltr, previous),
        _Bidi(' + ', null, sandwich),
        _Bidi('24', ltr, previous),
        _Bidi(' = ', null, sandwich),
        _Bidi('36', ltr, previous),
      ]);
    });

    test('handles new lines', () {
      expect(split('Lorem\n12\nipsum  \n'), <_Bidi>[
        _Bidi('Lorem', ltr, own),
        _Bidi('\n', null, sandwich),
        _Bidi('12', ltr, previous),
        _Bidi('\n', null, sandwich),
        _Bidi('ipsum', ltr, own),
        _Bidi('  \n', null, sandwich),
      ]);

      expect(split('$rtlWord1\n  $rtlWord2 \n'), <_Bidi>[
        _Bidi(rtlWord1, rtl, own),
        _Bidi('\n  ', null, sandwich),
        _Bidi(rtlWord2, rtl, own),
        _Bidi(' \n', null, sandwich),
      ]);
    });
  });
}

/// Holds information about how a bidi region was split from a string.
class _Bidi {
  _Bidi(this.text, this.textDirection, this.fragmentFlow);

  factory _Bidi.fromBidiFragment(String text, BidiFragment bidiFragment) {
    return _Bidi(
      text.substring(bidiFragment.start, bidiFragment.end),
      bidiFragment.textDirection,
      bidiFragment.fragmentFlow,
    );
  }

  final String text;
  final TextDirection? textDirection;
  final FragmentFlow fragmentFlow;

  @override
  int get hashCode => Object.hash(text, textDirection);

  @override
  bool operator ==(Object other) {
    return other is _Bidi &&
        other.text == text &&
        other.textDirection == textDirection &&
        other.fragmentFlow == fragmentFlow;
  }

  @override
  String toString() {
    return '"$text" ($textDirection | $fragmentFlow)';
  }
}

List<_Bidi> split(String text) {
  return <_Bidi>[
    for (final BidiFragment bidiFragment in BidiFragmenter(text).fragment())
      _Bidi.fromBidiFragment(text, bidiFragment)
  ];
}
