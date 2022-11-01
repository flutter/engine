import 'layout_types.dart';

const Map<String, String?> _kLayoutGoals = <String, String?>{
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
  'Minus': null,
  'Equal': null,
  'BracketLeft': null,
  'BracketRight': null,
  'Backslash': null,
  'Semicolon': null,
  'Quote': null,
  'Backquote': null,
  'Comma': null,
  'Period': null,
  'Slash': null,
};

final Map<String, String> _kMandatoryGoalsByChar = Map<String, String>.fromEntries(
  _kLayoutGoals
    .entries
    .where((MapEntry<String, String?> entry) => entry.value != null)
    .map((MapEntry<String, String?> entry) => MapEntry<String, String>(entry.value!, entry.key))
);

// Returns a mapping from eventCode to logical key for this layout.
Map<String, int> buildLayout(Map<String, LayoutEntry> entries) {
  // Unresolved mandatory goals, mapped from printables to KeyboardEvent.code.
  // This map will be modified during this function and thus is a clone.
  final Map<String, String> mandatoryGoalsByChar = <String, String>{..._kMandatoryGoalsByChar};
  // The result mapping from KeyboardEvent.code to logical key.
  final Map<String, int> result = <String, int>{};
  // The logical key should be the first available clue from below:
  //
  //  1. Mandatory goal, if it matches any clue. This ensures that all alnum
  //     keys can be found somewhere.
  //  2. US layout, if neither clue of the key is EASCII. This ensures that
  //     there are no non-latin logical keys.
  //  3. Derived on the fly from keyCode & characters.

  entries.forEach((String eventCode, LayoutEntry entry) {
    // bool anyEascii = false;
    for (int index = 0; index < 4; index += 1) {
      // Ignore dead keys.
      if (entry.deadMasks & (1 << index) != 0) {
        continue;
      }
      // A printable of this key is a mandatory goal: Use it.
      final String printable = entry.printables[index];
      if (mandatoryGoalsByChar.containsKey(printable)) {
        result[eventCode] = printable.codeUnitAt(0);
        mandatoryGoalsByChar.remove(printable);
      }
      // if (_isEascii(entry.printables[index])) {
      //   anyEascii = true;
      // }
    }

    // // If all clues on this key are non-EASCII, use the verbatim key.
    // if (!anyEascii) {
    //   final String? verbatimPrintable = kLayoutStore.goals[eventCode];
    //   if (verbatimPrintable != null
    //       && mandatoryGoalsByChar.remove(verbatimPrintable) != null) {
    //     result[eventCode] = verbatimPrintable.codeUnitAt(0);
    //   }
    // }
  });

  // Ensure all mandatory goals are assigned.
  mandatoryGoalsByChar.forEach((String character, String code) {
    result[code] = character.codeUnitAt(0);
  });
  // assert(() {
  //   print(result);
  //   return true;
  // }());
  return result;
}

final int _kLowerA = 'a'.codeUnitAt(0);
final int _kUpperA = 'A'.codeUnitAt(0);
final int _kLowerZ = 'z'.codeUnitAt(0);
final int _kUpperZ = 'Z'.codeUnitAt(0);
bool _charCodeIsLetter(int charCode) {
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ);
}

const int _kUseKeyCode = 1;

bool _mappedToKeyCode(int charCode) {
  return _charCodeIsLetter(charCode) || charCode == _kUseKeyCode;
}

// Return a map of EventCode -> EventKey -> logicalKey
Map<String, Map<String, int>> buildMap(Iterable<Layout> layouts) {
  final Map<String, Map<String, int>> result = <String, Map<String, int>>{};
  for (final Layout layout in layouts) {
    buildLayout(layout.entries).forEach((String eventCode, int logicalKey) {
      final Map<String, int> codeMap = result.putIfAbsent(eventCode, () => <String, int>{});
      final LayoutEntry entry = layout.entries[eventCode]!;
      for (int charIndex = 0; charIndex < 4; charIndex += 1) {
        final bool isDeadKey = entry.deadMasks & (1 << charIndex) != 0;
        final String printable = entry.printables[charIndex];
        if (!isDeadKey && printable.isEmpty) {
          continue;
        }
        final String eventKey = isDeadKey ? 'Deadkey' : printable;
        if (codeMap.containsKey(eventKey) && codeMap[eventKey] != logicalKey) {
          assert(_mappedToKeyCode(codeMap[eventKey]!));
          assert(_mappedToKeyCode(logicalKey));
          codeMap[eventKey] = _kUseKeyCode;
        } else {
          codeMap[eventKey] = logicalKey;
        }
      }
    });
  }
  return result;
}
