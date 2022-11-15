import 'common.dart';
import 'layout_types.dart';

// Maps all mandatory goals from the character to eventScanCode.
//
// Mandatory goals are all the alnum keys. These keys must be assigned at the
// end of layout planning.
final Map<String, String> _kMandatoryGoalsByChar = Map<String, String>.fromEntries(
  kLayoutGoals
    .entries
    .where((MapEntry<String, String> entry) => isAlnum(entry.value))
    .map((MapEntry<String, String?> entry) => MapEntry<String, String>(entry.value!, entry.key))
);

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
  return isLetterChar(charCode) || charCode == kUseKeyCode;
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
          assert(isLetterChar(logicalKey));
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
      heuristicDetector(eventCode, eventKey) == logicalKey,
    );
    return codeMap.isEmpty;
  });
  return result;
}
