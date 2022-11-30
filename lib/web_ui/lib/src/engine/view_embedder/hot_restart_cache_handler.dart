// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';
import '../safe_browser_api.dart';

/// Handles elements that need to be cleared after a hot-restart.
class HotRestartCacheHandler {
  HotRestartCacheHandler([this.storeName = '__flutter_state']) {
    if (_elements.isNotEmpty) {
      // We are in a post hot-restart world, clear the elements now.
      clearAllElements();
    }
  }

  /// This is state persistent across hot restarts that indicates what
  /// to clear.  Delay removal of old visible state to make the
  /// transition appear smooth.
  final String storeName;

  /// The js-interop layer backing [_elements].
  ///
  /// They're stored in a js global with name [storeName], and removed from the
  /// DOM when the app repaints...
  late List<DomElement?>? _jsElements;

  /// The elements that need to be cleaned up after hot-restart.
  List<DomElement?> get _elements {
    _jsElements = getJsProperty<List<DomElement?>?>(domWindow, storeName);
    if (_jsElements == null) {
      _jsElements = <DomElement?>[];
      setJsProperty(domWindow, storeName, _jsElements);
    }
    return _jsElements!;
  }

  void registerElement(DomElement element) {
    _elements.add(element);
  }

  void clearAllElements() {
    for (final DomElement? element in _elements) {
      element?.remove();
    }
    _elements.clear();
  }
}
