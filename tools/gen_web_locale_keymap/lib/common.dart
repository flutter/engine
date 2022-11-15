// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The following segment is not only used in the generating script, but also
// copied to the generated package.
/*@@@ SHARED SEGMENT START @@@*/

/// A special value in the final mapping that indicates the logical key should
/// be derived from KeyboardEvent.keyCode.
const int kUseKeyCode = 1;

/// A map of all goals from the scan codes to their mapped value in US layout.
const Map<String, String> kLayoutGoals = <String, String>{
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

/// Returns whether the `char` is a single character of a letter or a digit.
bool isAlnum(String char) {
  if (char.length != 1) {
    return false;
  }
  final int charCode = char.codeUnitAt(0);
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ)
      || (charCode >= _k0 && charCode <= _k9);
}

/// A set of rules that can derive a large number of logical keys simply from
/// the event's code and key.
///
/// This greatly reduces the entries needed in the final mapping.
int? heuristicMapper(String code, String key) {
  if (isAlnum(key)) {
    return key.toLowerCase().codeUnitAt(0);
  }
  if (!_isAscii(key)) {
    return kLayoutGoals[code]!.codeUnitAt(0);
  }
  return null;
}

/*@@@ SHARED SEGMENT END @@@*/

/// Whether the given charCode is a ASCII letter.
bool isLetterChar(int charCode) {
  return (charCode >= _kLowerA && charCode <= _kLowerZ)
      || (charCode >= _kUpperA && charCode <= _kUpperZ);
}
