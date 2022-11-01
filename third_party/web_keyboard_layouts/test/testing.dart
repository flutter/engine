// import 'package:litetest/litetest.dart';
import 'package:test/test.dart';
import 'package:web_keyboard_layouts/web_keyboard_layouts.dart';

void verifyEntry(LayoutMapping mapping, String eventCode, List<String> eventKeys, int logicalKey) {
  // If the KeyboardEvent.code is a letter key such as "KeyA", then
  // KeyboardEvent.keyCode is the upper letter such as "A". Otherwise, this
  // field must not be used (in reality this field may or may not be platform
  // independent).
  String? eventKeyCode;
  {
    final RegExp regexLetterKey = RegExp(r'^Key([A-Z])$');
    final RegExpMatch? matchLetterKey = regexLetterKey.firstMatch(eventCode);
    if (matchLetterKey != null) {
      eventKeyCode = matchLetterKey.group(1)!.toUpperCase();
    }
  }

  int index = 0;
  for (final String eventKey in eventKeys) {
    if (eventKey.isEmpty) {
      continue;
    }
    test('$eventCode $index', () {
      expect(mapping.getLogicalKey(eventCode, eventKey, eventKeyCode ?? ''), logicalKey);
    });
    index += 1;
  }
}
