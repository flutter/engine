import 'package:test/test.dart';
import 'package:web_keyboard_layouts/web_keyboard_layouts.dart';

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
