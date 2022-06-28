// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import '../html/paragraph/helper.dart';
import 'line_breaker_test_helper.dart';
import 'line_breaker_test_raw_data.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$LineBreakFragmenter', () {
    test('whitespace', () {
      expect(split('foo bar'), <Line>[
        Line('foo ', opportunity),
        Line('bar', endOfText),
      ]);
      expect(split('  foo    bar  '), <Line>[
        Line('  ', opportunity),
        Line('foo    ', opportunity),
        Line('bar  ', endOfText),
      ]);
    });

    test('single-letter lines', () {
      expect(split('foo a bar'), <Line>[
        Line('foo ', opportunity),
        Line('a ', opportunity),
        Line('bar', endOfText),
      ]);
      expect(split('a b c'), <Line>[
        Line('a ', opportunity),
        Line('b ', opportunity),
        Line('c', endOfText),
      ]);
      expect(split(' a b '), <Line>[
        Line(' ', opportunity),
        Line('a ', opportunity),
        Line('b ', endOfText),
      ]);
    });

    test('new line characters', () {
      final String bk = String.fromCharCode(0x000B);
      // Can't have a line break between CR×LF.
      expect(split('foo\r\nbar'), <Line>[
        Line('foo\r\n', mandatory),
        Line('bar', endOfText),
      ]);

      // Any other new line is considered a line break on its own.

      expect(split('foo\n\nbar'), <Line>[
        Line('foo\n', mandatory),
        Line('\n', mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo\r\rbar'), <Line>[
        Line('foo\r', mandatory),
        Line('\r', mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo$bk${bk}bar'), <Line>[
        Line('foo$bk', mandatory),
        Line(bk, mandatory),
        Line('bar', endOfText),
      ]);

      expect(split('foo\n\rbar'), <Line>[
        Line('foo\n', mandatory),
        Line('\r', mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo$bk\rbar'), <Line>[
        Line('foo$bk', mandatory),
        Line('\r', mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo\r${bk}bar'), <Line>[
        Line('foo\r', mandatory),
        Line(bk, mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo$bk\nbar'), <Line>[
        Line('foo$bk', mandatory),
        Line('\n', mandatory),
        Line('bar', endOfText),
      ]);
      expect(split('foo\n${bk}bar'), <Line>[
        Line('foo\n', mandatory),
        Line(bk, mandatory),
        Line('bar', endOfText),
      ]);

      // New lines at the beginning and end.

      expect(split('foo\n'), <Line>[
        Line('foo\n', mandatory),
        Line('', endOfText),
      ]);
      expect(split('foo\r'), <Line>[
        Line('foo\r', mandatory),
        Line('', endOfText),
      ]);
      expect(split('foo$bk'), <Line>[
        Line('foo$bk', mandatory),
        Line('', endOfText),
      ]);

      expect(split('\nfoo'), <Line>[
        Line('\n', mandatory),
        Line('foo', endOfText),
      ]);
      expect(split('\rfoo'), <Line>[
        Line('\r', mandatory),
        Line('foo', endOfText),
      ]);
      expect(split('${bk}foo'), <Line>[
        Line(bk, mandatory),
        Line('foo', endOfText),
      ]);

      // Whitespace with new lines.

      expect(split('foo  \n'), <Line>[
        Line('foo  \n', mandatory),
        Line('', endOfText),
      ]);

      expect(split('foo  \n   '), <Line>[
        Line('foo  \n', mandatory),
        Line('   ', endOfText),
      ]);

      expect(split('foo  \n   bar'), <Line>[
        Line('foo  \n', mandatory),
        Line('   ', opportunity),
        Line('bar', endOfText),
      ]);

      expect(split('\n  foo'), <Line>[
        Line('\n', mandatory),
        Line('  ', opportunity),
        Line('foo', endOfText),
      ]);
      expect(split('   \n  foo'), <Line>[
        Line('   \n', mandatory),
        Line('  ', opportunity),
        Line('foo', endOfText),
      ]);
    });

    test('trailing spaces and new lines', () {
      expect(
        computeLineBreakFragments('foo bar  '),
        const <LineBreakFragment>[
          LineBreakFragment(0, 4, opportunity, trailingNewlines: 0, trailingSpaces: 1),
          LineBreakFragment(4, 9, endOfText, trailingNewlines: 0, trailingSpaces: 2),
        ],
      );

      expect(
        computeLineBreakFragments('foo  \nbar\nbaz   \n'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 6, mandatory, trailingNewlines: 1, trailingSpaces: 3),
          LineBreakFragment(6, 10, mandatory, trailingNewlines: 1, trailingSpaces: 1),
          LineBreakFragment(10, 17, mandatory, trailingNewlines: 1, trailingSpaces: 4),
          LineBreakFragment(17, 17, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );
    });

    test('leading spaces', () {
      expect(
        computeLineBreakFragments(' foo'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 1, opportunity, trailingNewlines: 0, trailingSpaces: 1),
          LineBreakFragment(1, 4, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );

      expect(
        computeLineBreakFragments('   foo'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 3, opportunity, trailingNewlines: 0, trailingSpaces: 3),
          LineBreakFragment(3, 6, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );

      expect(
        computeLineBreakFragments('  foo   bar'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 2, opportunity, trailingNewlines: 0, trailingSpaces: 2),
          LineBreakFragment(2, 8, opportunity, trailingNewlines: 0, trailingSpaces: 3),
          LineBreakFragment(8, 11, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );

      expect(
        computeLineBreakFragments('  \n   foo'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 3, mandatory, trailingNewlines: 1, trailingSpaces: 3),
          LineBreakFragment(3, 6, opportunity, trailingNewlines: 0, trailingSpaces: 3),
          LineBreakFragment(6, 9, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );
    });

    test('whitespace before the last character', () {
      expect(
        computeLineBreakFragments('Lorem sit .'),
        const <LineBreakFragment>[
          LineBreakFragment(0, 6, opportunity, trailingNewlines: 0, trailingSpaces: 1),
          LineBreakFragment(6, 10, opportunity, trailingNewlines: 0, trailingSpaces: 1),
          LineBreakFragment(10, 11, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );
    });

    test('placeholders', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('Lorem');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('ipsum\n');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('dolor');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('\nsit');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
        },
      );

      final String placeholderChar = String.fromCharCode(0xFFFC);

      expect(splitParagraph(paragraph), <Line>[
        Line(placeholderChar, opportunity),
        Line('Lorem', opportunity),
        Line(placeholderChar, opportunity),
        Line('ipsum\n', mandatory),
        Line(placeholderChar, opportunity),
        Line('dolor', opportunity),
        Line('$placeholderChar\n', mandatory),
        Line('sit', opportunity),
        Line(placeholderChar, endOfText),
      ]);
    });

    test('placeholders surrounded by spaces and new lines', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('  Lorem  ');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('  \nipsum \n');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('\n');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
        },
      );

      expect(
        LineBreakFragmenter(paragraph).fragment(),
        const <LineBreakFragment>[
          LineBreakFragment(0, 3, opportunity, trailingNewlines: 0, trailingSpaces: 2),
          LineBreakFragment(3, 10, opportunity, trailingNewlines: 0, trailingSpaces: 2),
          LineBreakFragment(10, 14, mandatory, trailingNewlines: 1, trailingSpaces: 3),
          LineBreakFragment(14, 21, mandatory, trailingNewlines: 1, trailingSpaces: 2),
          LineBreakFragment(21, 23, mandatory, trailingNewlines: 1, trailingSpaces: 1),
          LineBreakFragment(23, 24, endOfText, trailingNewlines: 0, trailingSpaces: 0),
        ],
      );
    });

    test('comprehensive test', () {
      final List<TestCase> testCollection =
          parseRawTestData(rawLineBreakTestData);
      for (int t = 0; t < testCollection.length; t++) {
        final TestCase testCase = testCollection[t];

        final String text = testCase.toText();
        final List<LineBreakFragment> fragments = computeLineBreakFragments(text);

        // `f` is the index in the `fragments` list.
        int f = 0;
        LineBreakFragment currentFragment = fragments[f];

        int surrogateCount = 0;
        // `s` is the index in the `testCase.signs` list.
        for (int s = 0; s < testCase.signs.length - 1; s++) {
          // `i` is the index in the `text`.
          int i = s + surrogateCount;

          final Sign sign = testCase.signs[s];

          if (s < testCase.chars.length && testCase.chars[s].isSurrogatePair) {
            surrogateCount++;
            expect(
              currentFragment.end,
              greaterThan(i),
              reason: 'Failed at test case number $t:\n'
                  '${testCase.toString()}\n'
                  '"$text"\n'
                  '\nFragment ended in the middle of a surrogate pair at {${currentFragment.end}}.',
            );
          }

          i = s + surrogateCount;

          if (sign.isBreakOpportunity) {
            expect(
              currentFragment.end,
              i,
              reason: 'Failed at test case number $t:\n'
                  '${testCase.toString()}\n'
                  '"$text"\n'
                  '\nExpected fragment to end at {$i} but ended at {${currentFragment.end}}.',
            );
            currentFragment = fragments[++f];
          } else {
            expect(
              currentFragment.end,
              greaterThan(i),
              reason: 'Failed at test case number $t:\n'
                  '${testCase.toString()}\n'
                  '"$text"\n'
                  '\nFragment ended in early at {${currentFragment.end}}.',
            );
          }
        }

        // Now let's look at the last sign, which requires different handling.

        // The last line break is an endOfText (or a hard break followed by
        // endOfText if the last character is a hard line break).
        if (currentFragment.type == mandatory) {
          // When last character is a hard line break, there should be an
          // extra fragment to represent the empty line at the end.
          expect(
            fragments,
            hasLength(f + 2),
            reason: 'Failed at test case number $t:\n'
                '${testCase.toString()}\n'
                '"$text"\n'
                '\nExpected an extra fragment for endOfText but there wasn\'t one.',
          );

          currentFragment = fragments[++f];
        }

        expect(
          currentFragment.type,
          endOfText,
          reason: 'Failed at test case number $t:\n'
              '${testCase.toString()}\n'
              '"$text"\n\n'
              'Expected an endOfText fragment but found: $currentFragment',
        );
        expect(
          currentFragment.end,
          text.length,
          reason: 'Failed at test case number $t:\n'
              '${testCase.toString()}\n'
              '"$text"\n\n'
              'Expected an endOfText fragment ending at {${text.length}} but found: $currentFragment',
        );
      }
    });
  });
}

/// Holds information about how a line was split from a string.
class Line {
  Line(this.text, this.breakType);

  final String text;
  final LineBreakType breakType;

  factory Line.fromLineBreakFragment(String text, LineBreakFragment fragment) {
    return Line(
      text.substring(fragment.start, fragment.end),
      fragment.type,
    );
  }

  @override
  int get hashCode => Object.hash(text, breakType);

  @override
  bool operator ==(Object other) {
    return other is Line && other.text == text && other.breakType == breakType;
  }

  String get escapedText {
    final String bk = String.fromCharCode(0x000B);
    final String nl = String.fromCharCode(0x0085);
    return text
        .replaceAll('"', r'\"')
        .replaceAll('\n', r'\n')
        .replaceAll('\r', r'\r')
        .replaceAll(bk, '{BK}')
        .replaceAll(nl, '{NL}');
  }

  @override
  String toString() {
    return '"$escapedText" ($breakType)';
  }
}

List<Line> split(String text) {
  return <Line>[
    for (final LineBreakFragment fragment in computeLineBreakFragments(text))
      Line.fromLineBreakFragment(text, fragment)
  ];
}

List<Line> splitParagraph(CanvasParagraph paragraph) {
  return <Line>[
    for (final LineBreakFragment fragment in LineBreakFragmenter(paragraph).fragment())
      Line.fromLineBreakFragment(paragraph.toPlainText(), fragment)
  ];
}

List<LineBreakFragment> computeLineBreakFragments(String text) {
  return LineBreakFragmenter(plain(EngineParagraphStyle(), text)).fragment();
}
