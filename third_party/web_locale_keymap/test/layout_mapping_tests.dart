import 'package:test/test.dart';
import 'package:web_locale_keymap/web_locale_keymap.dart';

import 'test_cases.g.dart';

void main() {
  group('Win', () {
    testWin(LayoutMapping.win());
  });

  group('Linux', () {
    testLinux(LayoutMapping.linux());
  });

  group('Darwin', () {
    testDarwin(LayoutMapping.darwin());
  });
}
