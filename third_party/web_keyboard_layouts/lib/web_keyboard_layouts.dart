// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

library web_keyboard_layouts;

part 'key_mappings.g.dart';

const int _kUseKeyCode = 1;

class LayoutMapping {
  LayoutMapping.win() : _mapping = kWinMapping;
  LayoutMapping.linux() : _mapping = kLinuxMapping;
  LayoutMapping.darwin() : _mapping = kDarwinMapping;

  int? getLogicalKey(String eventCode, String eventKey, String eventKeyCode) {
    final int? result = _mapping[eventCode]?[eventKey];
    if (result == _kUseKeyCode) {
      return getLetterCode(eventKeyCode.toUpperCase());
    }
    return result;
  }

  final Map<String, Map<String, int>> _mapping;

  static int? getLetterCode(String letter) {
    assert (letter.length == 1);
    return letter.codeUnitAt(0);
  }
}
