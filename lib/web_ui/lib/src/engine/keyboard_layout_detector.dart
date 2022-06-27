// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:web_keyboard_layouts/web_keyboard_layouts.dart';
import 'dom.dart';

// Set this flag to true to see the details of detecting layouts.
const bool _debugLogLayoutEvents = false;

class KeyboardLayoutDetector {
  void update(DomKeyboardEvent event) {
    if (event.type != 'keydown') {
      return;
    }

    if (_candidates != null) {
      _filterCandidates(_candidates!, event);
      if (_candidates!.isNotEmpty) {
        return;
      } else {
        print('[Debug] Keyboard layout: Candidate exhausted.');
      }
    }

    _candidates = kLayouts;
    _filterCandidates(_candidates!, event);
    if (_candidates!.isEmpty) {
      _candidates = null;
    }
  }

  static void _filterCandidates(List<LayoutInfo> candidates, DomEvent event) {
  }

  List<LayoutInfo>? _candidates;

  static String _printEvent(DomKeyboardEvent event) {
    final String flags = <String>[
    ];
    return 'Event(${event.code}, ${event.key}, )';
  }
}
