// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

import 'key_mappings.g.dart';

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

int? _heuristicDetector(String code, String key) {
  if (_isAlnum(key)) {
    return key.toLowerCase().codeUnitAt(0);
  }
  if (!_isAscii(key)) {
    return _kFullLayoutGoals[code]!.codeUnitAt(0);
  }
  return null;
}

class LayoutMapping {
  LayoutMapping.win() : _mapping = kWinMapping;
  LayoutMapping.linux() : _mapping = kLinuxMapping;
  LayoutMapping.darwin() : _mapping = kDarwinMapping;

  static int? _characterToLogicalKey(String? key) {
    // We have yet to find a case where length >= 2 is useful.
    if (key == null || key.length >= 2) {
      return null;
    }
    final int result = key.toLowerCase().codeUnitAt(0);
    return result;
  }

  int? getLogicalKey(String? eventCode, String? eventKey, int eventKeyCode) {
    final int? result = _mapping[eventCode]?[eventKey];
    if (result == kUseKeyCode) {
      return eventKeyCode;
    }
    if (result == null) {
      final int? heuristicResult = _heuristicDetector(eventCode ?? '', eventKey ?? '');
      if (heuristicResult != null) {
        return heuristicResult;
      }
      // Characters: map to unicode zone.
      //
      // While characters are usually resolved in the last step, this can happen
      // in non-latin layouts when a non-latin character is on a symbol key (ru,
      // Semicolon-ж) or on an alnum key that has been assigned elsewhere (hu,
      // Digit0-Ö).
      final int? characterLogicalKey = _characterToLogicalKey(eventKey);
      if (characterLogicalKey != null) {
        return characterLogicalKey;
      }
    }
    return result;
  }

  final Map<String, Map<String, int>> _mapping;
}
