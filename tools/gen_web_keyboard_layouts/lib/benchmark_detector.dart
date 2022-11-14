import 'data.dart';
import 'layout_types.dart';

const int kUseKeyCode = 1;

const Map<String, String> _kFullLayoutGoals = <String, String>{
  'KeyA': 'A',
  'KeyB': 'B',
  'KeyC': 'C',
  'KeyD': 'D',
  'KeyE': 'E',
  'KeyF': 'F',
  'KeyG': 'G',
  'KeyH': 'H',
  'KeyI': 'I',
  'KeyJ': 'J',
  'KeyK': 'K',
  'KeyL': 'L',
  'KeyM': 'M',
  'KeyN': 'N',
  'KeyO': 'O',
  'KeyP': 'P',
  'KeyQ': 'Q',
  'KeyR': 'R',
  'KeyS': 'S',
  'KeyT': 'T',
  'KeyU': 'U',
  'KeyV': 'V',
  'KeyW': 'W',
  'KeyX': 'X',
  'KeyY': 'Y',
  'KeyZ': 'Z',
  'Digit1': '1',
  'Digit2': '2',
  'Digit3': '3',
  'Digit4': '4',
  'Digit5': '5',
  'Digit6': '6',
  'Digit7': '7',
  'Digit8': '8',
  'Digit9': '9',
  'Digit0': '0',
  'Minus': '-',
  'Equal': '=',
  'BracketLeft': '[',
  'BracketRight': ']',
  'Backslash': r'\',
  'Semicolon': ';',
  'Quote': "'",
  'Backquote': '`',
  'Comma': ',',
  'Period': '.',
  'Slash': '/',
};

final int _kLowerA = 'a'.codeUnitAt(0);
final int _kUpperA = 'A'.codeUnitAt(0);
final int _kLowerZ = 'z'.codeUnitAt(0);
final int _kUpperZ = 'Z'.codeUnitAt(0);
final int _k0 = '0'.codeUnitAt(0);
final int _k9 = '9'.codeUnitAt(0);

bool _isAscii(String key) {
  if (key.length != 1) {
    return false;
  }
  // 0x20 is the first printable character in ASCII.
  return key.codeUnitAt(0) >= 0x20 && key.codeUnitAt(0) <= 0x7F;
}

bool _isAlnum(String char) {
  if (char.length != 1) {
    return false;
  }
  final int charCode = char.codeUnitAt(0);
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ)
      || (charCode >= _k0 && charCode <= _k9);
}

bool _isLetterChar(int charCode) {
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ);
}

final Map<String, String> _kMandatoryGoalsByChar = Map<String, String>.fromEntries(
  kLayoutGoals
    .entries
    .where((MapEntry<String, String?> entry) => entry.value != null)
    .map((MapEntry<String, String?> entry) => MapEntry<String, String>(entry.value!, entry.key))
);

int? _heuristicDetector(String code, String key) {
  if (_isAlnum(key)) {
    return key.toLowerCase().codeUnitAt(0);
  }
  if (!_isAscii(key)) {
    return _kFullLayoutGoals[code]!.codeUnitAt(0);
  }
  return null;
}

/// Returns a mapping from eventCode to logical key for this layout.
///
/// If a eventCode does not exist in this map, then this event's logical key
/// should be derived on the fly.
Map<String, int> buildLayout(Map<String, LayoutEntry> entries) {
  // The logical key is derived in the following rules:
  //
  //  1. If any clue (the four possible printables) of the key is a mandatory
  //     goal (alnum), then the goal is the logical key.
  //  2. If a mandatory goal is not assigned in the way of #1, then it is
  //     assigned to the physical key as mapped in the US layout.
  //  3. Derived on the fly from key, code, and keyCode.
  //
  // The map returned from this function contains the first two rules.

  // Unresolved mandatory goals, mapped from printables to KeyboardEvent.code.
  // This map will be modified during this function and thus is a clone.
  final Map<String, String> mandatoryGoalsByChar = <String, String>{..._kMandatoryGoalsByChar};
  // The result mapping from KeyboardEvent.code to logical key.
  final Map<String, int> result = <String, int>{};

  entries.forEach((String eventCode, LayoutEntry entry) {
    for (final String printable in entry.printables) {
      if (mandatoryGoalsByChar.containsKey(printable)) {
        result[eventCode] = printable.codeUnitAt(0);
        mandatoryGoalsByChar.remove(printable);
        break;
      }
    }
  });

  // Ensure all mandatory goals are assigned.
  mandatoryGoalsByChar.forEach((String character, String code) {
    assert(!result.containsKey(code), 'Code $code conflicts.');
    result[code] = character.codeUnitAt(0);
  });
  return result;
}

bool _isLetterOrMappedToKeyCode(int charCode) {
  return _isLetterChar(charCode) || charCode == kUseKeyCode;
}

/// Summarize all layouts into a huge table of EventCode -> EventKey ->
/// logicalKey.
///
/// The resulting logicalKey can also be kUseKeyCode.
///
/// If a eventCode does not exist in this map, then this event's logical key
/// should be derived on the fly.
Map<String, Map<String, int>> buildMap(Iterable<Layout> layouts) {
  final Map<String, Map<String, int>> result = <String, Map<String, int>>{};
  for (final Layout layout in layouts) {
    buildLayout(layout.entries).forEach((String eventCode, int logicalKey) {
      final Map<String, int> codeMap = result.putIfAbsent(eventCode, () => <String, int>{});
      final LayoutEntry entry = layout.entries[eventCode]!;
      for (final String eventKey in entry.printables) {
        if (eventKey.isEmpty) {
          continue;
        }
        // Found conflict. Assert that all such cases can be solved with
        // keyCode.
        if (codeMap.containsKey(eventKey) && codeMap[eventKey] != logicalKey) {
          assert(_isLetterChar(logicalKey));
          assert(_isLetterOrMappedToKeyCode(codeMap[eventKey]!), '$eventCode, $eventKey, ${codeMap[eventKey]!}');
          codeMap[eventKey] = kUseKeyCode;
        } else {
          codeMap[eventKey] = logicalKey;
        }
      }
    });
  }
  // Remove mapping results that can be derived using heuristics.
  result.removeWhere((String eventCode, Map<String, int> codeMap) {
    codeMap.removeWhere((String eventKey, int logicalKey) =>
      _heuristicDetector(eventCode, eventKey) == logicalKey,
    );
    return codeMap.isEmpty;
  });
  return result;
}
