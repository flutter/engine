// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';

import '../html/paragraph/helper.dart';

final EngineTextStyle defaultStyle = EngineTextStyle.only();
final EngineTextStyle style1 = EngineTextStyle.only(fontSize: 20);
final EngineTextStyle style2 = EngineTextStyle.only(color: blue);
final EngineTextStyle style3 = EngineTextStyle.only(fontFamily: 'Roboto');

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  group('$LayoutFragmenter', () {
    test('single span', () {
      final CanvasParagraph paragraph =
          plain(EngineParagraphStyle(), 'Lorem 12 $rtlWord1   ipsum34');
      expect(split(paragraph), <_Fragment>[
        _Fragment('Lorem ', opportunity, ltr, defaultStyle, sp: 1),
        _Fragment('12 ', opportunity, ltr, defaultStyle, sp: 1),
        _Fragment('$rtlWord1   ', opportunity, rtl, defaultStyle, sp: 3),
        _Fragment('ipsum34', opportunity, ltr, defaultStyle),
      ]);
    });

    test('multi span', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.pushStyle(style1);
          builder.addText('Lorem');
          builder.pop();
          builder.pushStyle(style2);
          builder.addText(' ipsum 12 ');
          builder.pop();
          builder.pushStyle(style3);
          builder.addText(' $rtlWord1 foo.');
          builder.pop();
        },
      );

      expect(split(paragraph), <_Fragment>[
        _Fragment('Lorem', prohibited, ltr, style1),
        _Fragment(' ', opportunity, ltr, style2, sp: 1),
        _Fragment('ipsum ', opportunity, ltr, style2, sp: 1),
        _Fragment('12 ', prohibited, ltr, style2, sp: 1),
        _Fragment(' ', opportunity, ltr, style3, sp: 1),
        _Fragment('$rtlWord1 ', opportunity, rtl, style3, sp: 1),
        _Fragment('foo.', endOfText, ltr, style3),
      ]);
    });

    test('new lines', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.pushStyle(style1);
          builder.addText('Lor\nem \n');
          builder.pop();
          builder.pushStyle(style2);
          builder.addText(' \n  ipsum 12 ');
          builder.pop();
          builder.pushStyle(style3);
          builder.addText(' $rtlWord1 fo');
          builder.pop();
          builder.pushStyle(style1);
          builder.addText('o.');
          builder.pop();
        },
      );

      expect(split(paragraph), <_Fragment>[
        _Fragment('Lor\n', mandatory, ltr, style1, nl: 1, sp: 1),
        _Fragment('em \n', mandatory, ltr, style1, nl: 1, sp: 2),
        _Fragment(' \n', mandatory, ltr, style2, nl: 1, sp: 2),
        _Fragment('  ', opportunity, ltr, style2, sp: 2),
        _Fragment('ipsum ', opportunity, ltr, style2, sp: 1),
        _Fragment('12 ', prohibited, ltr, style2, sp: 1),
        _Fragment(' ', opportunity, ltr, style3, sp: 1),
        _Fragment('$rtlWord1 ', opportunity, rtl, style3, sp: 1),
        _Fragment('fo', prohibited, ltr, style3),
        _Fragment('o.', endOfText, ltr, style1),
      ]);
    });

    test('last line is empty', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.pushStyle(style1);
          builder.addText('Lorem \n');
          builder.pop();
          builder.pushStyle(style2);
          builder.addText(' \n  ipsum \n');
          builder.pop();
        },
      );

      expect(split(paragraph), <_Fragment>[
        _Fragment('Lorem \n', mandatory, ltr, style1, nl: 1, sp: 2),
        _Fragment(' \n', mandatory, ltr, style2, nl: 1, sp: 2),
        _Fragment('  ', opportunity, ltr, style2, sp: 2),
        _Fragment('ipsum \n', mandatory, ltr, style2, nl: 1, sp: 2),
        _Fragment('', endOfText, ltr, style2),
      ]);
    });

    test('placeholders', () {
      final CanvasParagraph paragraph = rich(
        EngineParagraphStyle(),
        (CanvasParagraphBuilder builder) {
          builder.pushStyle(style1);
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('Lorem');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('ipsum\n');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.pop();
          builder.pushStyle(style2);
          builder.addText('$rtlWord1 ');
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
          builder.addText('\nsit');
          builder.pop();
          builder.addPlaceholder(100, 100, PlaceholderAlignment.top);
        },
      );

      final String placeholderChar = String.fromCharCode(0xFFFC);

      expect(split(paragraph), <_Fragment>[
        _Fragment(placeholderChar, opportunity, ltr, null),
        _Fragment('Lorem', opportunity, ltr, style1),
        _Fragment(placeholderChar, opportunity, ltr, null),
        _Fragment('ipsum\n', mandatory, ltr, style1, nl: 1, sp: 1),
        _Fragment(placeholderChar, opportunity, ltr, null),
        _Fragment('$rtlWord1 ', opportunity, rtl, style2, sp: 1),
        _Fragment(placeholderChar, prohibited, ltr, null),
        _Fragment('\n', mandatory, ltr, style2, nl: 1, sp: 1),
        _Fragment('sit', opportunity, ltr, style2),
        _Fragment(placeholderChar, endOfText, ltr, null),
      ]);
    });
  });
}

/// Holds information about how a fragment.
class _Fragment {
  _Fragment(this.text, this.type, this.textDirection, this.style, {
    this.nl = 0,
    this.sp = 0,
  });

  final String text;
  final LineBreakType type;
  final TextDirection textDirection;
  final EngineTextStyle? style;

  /// The number of trailing new line characters.
  final int nl;

  /// The number of trailing spaces.
  final int sp;

  factory _Fragment._fromLayoutFragment(String text, LayoutFragment layoutFragment) {
    final ParagraphSpan span = layoutFragment.span;
    return _Fragment(
      text.substring(layoutFragment.start, layoutFragment.end),
      layoutFragment.type,
      layoutFragment.textDirection,
      span is FlatTextSpan ? span.style : null,
      nl: layoutFragment.trailingNewlines,
      sp: layoutFragment.trailingSpaces,
    );
  }

  @override
  int get hashCode => Object.hash(text, textDirection);

  @override
  bool operator ==(Object other) {
    return other is _Fragment &&
        other.text == text &&
        other.textDirection == textDirection;
  }

  @override
  String toString() {
    return '"$text" ($textDirection)';
  }
}

List<_Fragment> split(CanvasParagraph paragraph) {
  return <_Fragment>[
    for (final LayoutFragment layoutFragment
        in computeLayoutFragments(paragraph))
      _Fragment._fromLayoutFragment(paragraph.plainText, layoutFragment)
  ];
}

List<LayoutFragment> computeLayoutFragments(CanvasParagraph paragraph) {
  return LayoutFragmenter(paragraph).fragment();
}
