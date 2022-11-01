// import 'package:litetest/litetest.dart';
import 'package:test/test.dart';
import 'package:web_keyboard_layouts/web_keyboard_layouts.dart';

final int _kLowerA = 'a'.codeUnitAt(0);
final int _kUpperA = 'A'.codeUnitAt(0);
final int _kLowerZ = 'z'.codeUnitAt(0);
final int _kUpperZ = 'Z'.codeUnitAt(0);

bool _isLetter(String char) {
  if (char.length != 1) {
    return false;
  }
  final int charCode = char.codeUnitAt(0);
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ);
}

String _fromCharCode(int? logicalKey) {
  if (logicalKey == null) {
    return '';
  }
  return String.fromCharCode(logicalKey);
}

void verifyEntry(LayoutMapping mapping, String eventCode, List<String> eventKeys, String mappedResult) {
  // If the first two entry of KeyboardEvent.key are letter keys such as "a" and
  // "A", then KeyboardEvent.keyCode is the upper letter such as "A". Otherwise,
  // this field must not be used (in reality this field may or may not be
  // platform independent).
  String? eventKeyCode;
  {
    if (_isLetter(eventKeys[0]) && _isLetter(eventKeys[1])) {
      eventKeyCode = eventKeys[0].toUpperCase();
    }
  }

  int index = 0;
  for (final String eventKey in eventKeys) {
    if (eventKey.isEmpty) {
      continue;
    }
    test('$eventCode $index', () {
      expect(
        _fromCharCode(mapping.getLogicalKey(eventCode, eventKey, eventKeyCode ?? '')),
        mappedResult,
      );
    });
    index += 1;
  }
}
