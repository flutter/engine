/*---------------------------------------------------------------------------------------------
 *  Copyright (c) 2022 Google LLC
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import 'key_mappings.g.dart';

class LayoutMapping {
  LayoutMapping.win() : _mapping = kMappingDataWin;
  LayoutMapping.linux() : _mapping = kMappingDataLinux;
  LayoutMapping.darwin() : _mapping = kMappingDataDarwin;

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
      final int? heuristicResult = heuristicDetector(eventCode ?? '', eventKey ?? '');
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
