// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:web_keyboard_layouts/web_keyboard_layouts.dart' as keyboard_layouts;
import 'dom.dart';

// Set this flag to true to see the details of detecting layouts.
const bool _debugLogLayoutEvents = false;
void _debugLog(String message) {
  if (_debugLogLayoutEvents) {
    print(_debugLogLayoutEvents);
  }
}

bool _isEascii(int clue) {
  // This also excludes kDeadKey.
  return clue < 256;
}

class KeyboardLayoutDetector {
  KeyboardLayoutDetector() {
    int index = 0;
    for (final String key in keyboard_layouts.kLayoutGoals.keys) {
      _goalToIndex[key] = index;
      index += 1;
    }
  }

  void update(DomKeyboardEvent event) {
    if (event.type != 'keydown') {
      return;
    }
    final int? goalIndex = _goalToIndex[event.code];
    if (goalIndex == null) {
      return;
    }

    // There is an existing candidate list. Filter based on it.
    if (_candidates.isEmpty) {
      final bool effectiveCue = _filterCandidates(_candidates, event, goalIndex);
      if (effectiveCue) {
        _debugCues.add(event);
      }
      if (_candidates.isNotEmpty) {
        // There are some candidates left. Move forward.
        return;
      } else {
        _debugLog('[Debug] Keyboard layout: Candidates exhausted. Past cues:'
          + _debugCuesToString(_debugCues));
      }
    }

    // Start anew: Filter based on the entire list.
    _candidates.addAll(_fullCandidates);
    _debugCues.clear();
    final bool effectiveCue = _filterCandidates(_candidates, event, goalIndex);
    if (effectiveCue) {
      _debugCues.add(event);
    }
    if (_candidates.isEmpty) {
      _debugLog('[Debug] Keyboard layout: Candidates exhausted on first try. Past cues:'
        + _debugCuesToString(_debugCues));
    }
  }

  int? getKey(String code) {
    if (_candidates.isEmpty) {
      return null;
    }
    final keyboard_layouts.LayoutInfo candidate = _candidates.first;
    if (!_calculatedLayouts.containsKey(candidate.name)) {
      _calculatedLayouts[candidate.name] = _buildLayout(candidate.mapping, candidate.name);
    }
    final Map<String, int> map = _calculatedLayouts[candidate.name]!;
    return map[code];
  }

  static bool _filterCandidates(List<keyboard_layouts.LayoutInfo> candidates, DomKeyboardEvent event, int goalIndex) {
    final int beforeCandidateNum = candidates.length;
    final bool thisIsDead = event.key == 'Dead';
    final bool thisHasAltGr = event.getModifierState('AltGraph');
    final bool thisHasShift = event.shiftKey;
    final int index = (thisHasShift ? 1 : 0) + (thisHasAltGr ? 2 : 0);
    candidates.where((keyboard_layouts.LayoutInfo element) {
      final int expected = element.mapping[goalIndex][index];
      if (thisIsDead) {
        return expected == keyboard_layouts.kDeadKey;
      } else {
        final String key = event.key ?? '';
        // TODO: Correctly process Utf16
        return key.length == 1 && key.codeUnitAt(0) == expected;
      }
    });
    final int afterCandidateNum = candidates.length;
    return afterCandidateNum < beforeCandidateNum;
  }

  static Map<String, int> _buildLayout(List<List<int>> clueMap, String debugLayoutName) {
    assert(() {
      print('Building layout for $debugLayoutName');
      return true;
    }());
    final Map<int, String> mandatoryGoalsByChar = <int, String>{..._mandatoryGoalsByChar};
    final Map<String, int> result = <String, int>{};
    // The logical key should be the first available clue from below:
    //
    //  - Mandatory goal, if it matches any clue. This ensures that all alnum
    //    keys can be found somewhere.
    //  - US layout, if neither clue of the key is EASCII. This ensures that
    //    there are no non-latin logical keys.
    //  - Derived on the fly from keyCode & characters.
    int goalIndex = 0;
    keyboard_layouts.kLayoutGoals.forEach((String code, String? goalKey) {
      // Skip optional goals.
      if (goalKey == null) {
        return;
      }
      final List<int> clues = clueMap[goalIndex];
      // See if any clue on this key matches a mandatory char.
      for (final int clue in clues) {
        final String? foundCode = mandatoryGoalsByChar[clue];
        if (foundCode != null) {
          result[code] = clue;
          mandatoryGoalsByChar.remove(foundCode);
          return;
        }
      }
      // See if all clues on this key are non-EASCII. If not, use the verbatim key.
      if (!clues.any(_isEascii)) {
        final int character = keyboard_layouts.kLayoutGoals[code]!.codeUnitAt(0);
        result[code] = character;
        mandatoryGoalsByChar.remove(character);
      }

      goalIndex += 1;
    });

    // Ensure all mandatory goals are assigned.
    mandatoryGoalsByChar.forEach((int character, String code) {
      result[code] = character;
    });
    assert(() {
      print(result);
      return true;
    }());
    return result;
  }

  final Map<String, int> _goalToIndex = <String, int>{};
  final List<keyboard_layouts.LayoutInfo> _candidates = <keyboard_layouts.LayoutInfo>[];
  final Map<String, Map<String, int>> _calculatedLayouts = <String, Map<String, int>>{};
  // Record all effective cues since the last reset. That is, cues that filtered
  // out any candidates. This is used to print out debug information.
  final List<DomKeyboardEvent> _debugCues = <DomKeyboardEvent>[];

  static late final List<keyboard_layouts.LayoutInfo> _fullCandidates = keyboard_layouts.kLayouts.where(
    (keyboard_layouts.LayoutInfo layout) => layout.platform == _currentPlatform,
  ).toList();
  static late final keyboard_layouts.LayoutPlatform _currentPlatform = () {
    // TODO
    return keyboard_layouts.LayoutPlatform.win;
  }();
  static late final Map<int, String> _mandatoryGoalsByChar = Map<int, String>.fromEntries(
    keyboard_layouts.kLayoutGoals
      .entries
      .where((MapEntry<String, String?> entry) => entry.value != null)
      // TODO: correctly handle UTF
      .map((MapEntry<String, String?> entry) => MapEntry<int, String>(entry.value!.codeUnitAt(0), entry.key))
  );

  static String _debugEventToString(DomKeyboardEvent event) {
    final String flags = <String>[
      if (event.altKey) 'Alt',
      if (event.shiftKey) 'Shift',
    ].join(' | ');
    final String parameters = <String>[
      event.code ?? '',
      event.key ?? '',
      if (flags.isNotEmpty) flags,
    ].join(', ');
    return 'Event($parameters)';
  }

  static String _debugCuesToString(List<DomKeyboardEvent> cues) {
    return cues.map((DomKeyboardEvent event) =>
        '\n  ${_debugEventToString(event)}').join('');
  }
}
