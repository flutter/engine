// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../dom.dart';

/// Sets global attributes on [_hostElement] (the body element in full page mode).
///
/// The global attributes provide quick and general information about the
/// Flutter app. They are set on a global element (e.g. the body element) to
/// make it easily accessible to the user.
class GlobalHtmlAttributes {
  GlobalHtmlAttributes(this._hostElement);

  final DomElement _hostElement;

  void applyAttributes({
    required bool autoDetectRenderer,
    required String rendererTag,
    required String buildMode,
  }) {
    // How was the current renderer selected?
    final String rendererSelection = autoDetectRenderer
        ? 'auto-selected'
        : 'requested explicitly';

    _hostElement.setAttribute('flt-renderer', '$rendererTag ($rendererSelection)');
    _hostElement.setAttribute('flt-build-mode', buildMode);
    // TODO(mdebbar): Disable spellcheck until changes in the framework and
    // engine are complete.
    _hostElement.setAttribute('spellcheck', 'false');
  }
}
