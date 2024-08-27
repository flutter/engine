// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import 'label_and_value.dart';
import 'semantics.dart';

/// Renders semantics objects that have selected (true/false) states.
///
/// See also [ui.SemanticsFlag.isSelected] & [ui.SemanticsFlag.hasSelectedState]
class SemanticsClickSelectable extends SemanticRole {
  SemanticsClickSelectable(SemanticsObject semanticsObject) : super.withBasics(
    SemanticRoleKind.click_selectable,
    semanticsObject,
    preferredLabelRepresentation: LabelRepresentation.ariaLabel,) {
      addTappable();
  }

  @override
  void update() {
    super.update();

    if (semanticsObject.isFlagsDirty && semanticsObject.hasFlag(ui.SemanticsFlag.hasSelectedState)) {
      setAriaRole('tab');

      /// Adding disabled and aria-disabled attribute to notify the assistive
      /// technologies of disabled elements.
      _updateDisabledAttribute();

      setAttribute(
        'aria-selected',
        (semanticsObject.hasFlag(ui.SemanticsFlag.hasSelectedState) &&
        semanticsObject.hasFlag(ui.SemanticsFlag.isSelected) &&
        semanticsObject.hasFlag(ui.SemanticsFlag.isFocusable))
            ? 'true'
            : 'false',
      );
    }
  }

  @override
  void dispose() {
    super.dispose();
    _removeDisabledAttribute();
  }

  void _updateDisabledAttribute() {
    if (semanticsObject.enabledState() == EnabledState.disabled) {
      setAttribute('aria-disabled', 'true');
      setAttribute('disabled', 'true');
    } else {
      _removeDisabledAttribute();
    }
  }

  void _removeDisabledAttribute() {
    removeAttribute('aria-disabled');
    removeAttribute('disabled');
  }

  @override
  bool focusAsRouteDefault() => focusable?.focusAsRouteDefault() ?? false;
}
