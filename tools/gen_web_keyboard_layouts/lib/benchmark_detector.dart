import 'data.dart';
import 'layout_types.dart';

const int kUseKeyCode = 1;

final int _kLowerA = 'a'.codeUnitAt(0);
final int _kUpperA = 'A'.codeUnitAt(0);
final int _kLowerZ = 'z'.codeUnitAt(0);
final int _kUpperZ = 'Z'.codeUnitAt(0);

bool _isEascii(String key) {
  if (key.length != 1) {
    return false;
  }
  // 0x20 is the first printable character in ASCII.
  return key.codeUnitAt(0) >= 0x20 && key.codeUnitAt(0) <= 0xFF;
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

/// Returns a mapping from eventCode to logical key for this layout.
///
/// If a eventCode does not exist in this map, then this event's logical key
/// should be derived on the fly.
Map<String, int> buildLayout(Map<String, LayoutEntry> entries) {
  // Unresolved mandatory goals, mapped from printables to KeyboardEvent.code.
  // This map will be modified during this function and thus is a clone.
  final Map<String, String> mandatoryGoalsByChar = <String, String>{..._kMandatoryGoalsByChar};
  // The result mapping from KeyboardEvent.code to logical key.
  final Map<String, int> result = <String, int>{};
  // The logical key is derived in the following rules:
  //
  //  1. If any clue (the four possible printables) of the key is a mandatory
  //     goal (alnum), then the goal is the logical key.
  //  2. If a mandatory goal is not assigned in the way of #1, then it is
  //     assigned to the physical key as mapped in the US layout.
  //  3. Derived on the fly from key, code, and keyCode.

  entries.forEach((String eventCode, LayoutEntry entry) {
    bool matchedMandatoryGoal = false;
    for (final String printable in entry.printables) {
      if (mandatoryGoalsByChar.containsKey(printable)) {
        result[eventCode] = printable.codeUnitAt(0);
        mandatoryGoalsByChar.remove(printable);
        matchedMandatoryGoal = true;
        break;
      }
    }
    if (!matchedMandatoryGoal &&
        !_isEascii(entry.printables[0]) &&
        !_isEascii(entry.printables[1])) {
      final String? verbatimPrintable = kLayoutGoals[eventCode];
      if (verbatimPrintable != null
          && mandatoryGoalsByChar.remove(verbatimPrintable) != null) {
        result[eventCode] = verbatimPrintable.codeUnitAt(0);
      }
    }
  });

  // Ensure all mandatory goals are assigned.
  mandatoryGoalsByChar.forEach((String character, String code) {
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
        if (codeMap.containsKey(eventKey) && codeMap[eventKey] != logicalKey) {
          assert(_isLetterChar(logicalKey));
          assert(_isLetterOrMappedToKeyCode(codeMap[eventKey]!));
          codeMap[eventKey] = kUseKeyCode;
        } else {
          codeMap[eventKey] = logicalKey;
        }
      }
    });
  }
  return result;
}
