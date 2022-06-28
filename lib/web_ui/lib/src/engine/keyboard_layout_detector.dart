// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:web_keyboard_layouts/web_keyboard_layouts.dart';
import 'dom.dart';

// Set this flag to true to see the details of detecting layouts.
const bool _debugLogLayoutEvents = false;
void _debugLog(String message) {
  if (_debugLogLayoutEvents) {
    print(_debugLogLayoutEvents);
  }
}

class KeyboardLayoutDetector {
  void update(DomKeyboardEvent event) {
    if (event.type != 'keydown') {
      return;
    }

    // There is an existing candidate list. Filter based on it.
    if (_candidates.isEmpty) {
      final bool validCue = _filterCandidates(_candidates, event);
      if (validCue) {
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
    _candidates.addAll(kLayouts);
    _debugCues.clear();
    final bool validCue = _filterCandidates(_candidates, event);
    if (validCue) {
      _debugCues.add(event);
    }
    if (_candidates.isEmpty) {
      _debugLog('[Debug] Keyboard layout: Candidates exhausted on first try. Past cues:'
        + _debugCuesToString(_debugCues));
    }
  }

  static bool _filterCandidates(List<LayoutInfo> candidates, DomKeyboardEvent event) {
    final int beforeCandidateNum = candidates.length;
    final bool thisIsDead = event.key == 'Dead';
    final bool thisHasAltGr = event.getModifierState('AltGraph');
    final bool thisHasShift = event.shiftKey;
    candidates.where((LayoutInfo element) {
      final bool candidateIsDead =
    });
    final int afterCandidateNum = candidates.length;
    return afterCandidateNum < beforeCandidateNum;
  }

  final List<LayoutInfo> _candidates = <LayoutInfo>[];
  final List<DomKeyboardEvent> _debugCues = <DomKeyboardEvent>[];

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
