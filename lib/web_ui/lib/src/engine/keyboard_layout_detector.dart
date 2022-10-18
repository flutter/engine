// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:web_keyboard_layouts/web_keyboard_layouts.dart' as keyboard_layouts;
import 'dom.dart';

// Set this flag to true to see the details during layout detection.
const bool _debugLogLayoutEvents = true;
void _debugLog(String message) {
  if (_debugLogLayoutEvents) {
    print(message);
  }
}

// bool _isEascii(String clue) {
//   return clue.length == 1 && clue.codeUnitAt(0) < 256;
// }

class KeyboardLayoutDetector {
  void update(DomKeyboardEvent event) {
    if (event.type != 'keydown') {
      return;
    }
    if (!keyboard_layouts.kLayoutStore.goals.containsKey(event.code)) {
      return;
    }
    print('Cue ${event.code} key ${event.key}');

    // There is an existing candidate list. Filter based on it.
    if (_candidates.isNotEmpty) {
      final bool effectiveCue = _filterCandidates(_candidates, event);
      if (effectiveCue) {
        _debugCues.add(event);
      }
      if (_candidates.isNotEmpty) {
        // There are some candidates left. Move forward.
        return;
      } else {
        _debugLog('[Debug] Keyboard layout: Candidates exhausted. Past cues:'
          '${_debugCuesToString(_debugCues)}');
      }
    }

    // Start anew: Filter based on the entire list.
    _candidates.addAll(_fullCandidates);
    _debugCues.clear();
    final bool effectiveCue = _filterCandidates(_candidates, event);
    if (effectiveCue) {
      _debugCues.add(event);
    }
    if (_candidates.isEmpty) {
      _debugLog('[Debug] Keyboard layout: Candidates exhausted on first try. Past cues:'
        '${_debugCuesToString(_debugCues)}');
    }
  }

  int? getKey(String code) {
    if (_candidates.isEmpty) {
      return null;
    }
    final keyboard_layouts.Layout candidate = _candidates.first;
    if (candidate.language != _debugLastLayout) {
      _debugLog('[Debug] Switching to layout ${candidate.language}.');
      _debugLastLayout = candidate.language;
    }
    final Map<String, int> map = _calculatedLayouts.putIfAbsent(
        candidate.language,
        () => _buildLayout(candidate.entries, candidate.language));
    return map[code];
  }

  static bool _filterCandidates(List<keyboard_layouts.Layout> candidates, DomKeyboardEvent event) {
    final int beforeCandidateNum = candidates.length;
    final bool thisIsDead = event.key == 'Dead';
    final bool thisHasAltGr = event.getModifierState('AltGraph');
    final bool thisHasShift = event.shiftKey;
    final int printableIndex = (thisHasShift ? 1 : 0) + (thisHasAltGr ? 2 : 0);
    final int deadMask = 1 << printableIndex;
    candidates.retainWhere((keyboard_layouts.Layout layout) {
      final keyboard_layouts.LayoutEntry entry = layout.entries[event.code!]!;
      if (thisIsDead) {
        return (entry.deadMasks & deadMask) != 0;
      } else {
        return entry.printables[printableIndex] == event.key;
      }
    });
    final int afterCandidateNum = candidates.length;
    print('Filter before $beforeCandidateNum after $afterCandidateNum');
    return afterCandidateNum < beforeCandidateNum;
  }

  static Map<String, int> _buildLayout(Map<String, keyboard_layouts.LayoutEntry> entries, String debugLayoutName) {
    _debugLog('Building layout for $debugLayoutName');
    // Unresolved mandatory goals, mapped from printables to KeyboardEvent.code.
    // This map will be modified during this function and thus is a clone.
    final Map<String, String> mandatoryGoalsByChar = <String, String>{..._mandatoryGoalsByChar};
    // The result mapping from KeyboardEvent.code to logical key.
    final Map<String, int> result = <String, int>{};
    // The logical key should be the first available clue from below:
    //
    //  1. Mandatory goal, if it matches any clue. This ensures that all alnum
    //     keys can be found somewhere.
    //  2. US layout, if neither clue of the key is EASCII. This ensures that
    //     there are no non-latin logical keys.
    //  3. Derived on the fly from keyCode & characters.

    entries.forEach((String eventCode, keyboard_layouts.LayoutEntry entry) {
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
    assert(() {
      print(result);
      return true;
    }());
    return result;
  }

  final List<keyboard_layouts.Layout> _candidates = <keyboard_layouts.Layout>[];
  final Map<String, Map<String, int>> _calculatedLayouts = <String, Map<String, int>>{};

  // Record all effective cues since the last reset. That is, cues that filtered
  // out any candidates. This is used to print out debug information.
  final List<DomKeyboardEvent> _debugCues = <DomKeyboardEvent>[];
  String _debugLastLayout = '';

  static final List<keyboard_layouts.Layout> _fullCandidates = keyboard_layouts.kLayoutStore.layouts.where(
    (keyboard_layouts.Layout layout) => layout.platform == _currentPlatform,
  ).toList();
  static final keyboard_layouts.LayoutPlatform _currentPlatform = () {
    // TODO
    return keyboard_layouts.LayoutPlatform.win;
  }();
  static final Map<String, String> _mandatoryGoalsByChar = Map<String, String>.fromEntries(
    keyboard_layouts.kLayoutStore.goals
      .entries
      .where((MapEntry<String, String?> entry) => entry.value != null)
      .map((MapEntry<String, String?> entry) => MapEntry<String, String>(entry.value!, entry.key))
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
        '\n  ${_debugEventToString(event)}').join();
  }
}
