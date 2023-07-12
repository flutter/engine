// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// Sets the "button" ARIA role.
class Button extends PrimaryRoleManager {
  Button(SemanticsObject semanticsObject) : super.withBasics(PrimaryRole.button, semanticsObject) {
    semanticsObject.setAriaRole('button');
  }

  @override
  void update() {
    super.update();

    if (semanticsObject.enabledState() == EnabledState.disabled) {
      semanticsObject.element.setAttribute('aria-disabled', 'true');
    } else {
      semanticsObject.element.removeAttribute('aria-disabled');
    }
  }
}

/// Listens to HTML "click" gestures detected by the browser.
///
/// This gestures is different from the click and tap gestures detected by the
/// framework from raw pointer events. When an assistive technology is enabled
/// the browser may not send us pointer events. In that mode we forward HTML
/// click as [ui.SemanticsAction.tap].
class Tappable extends RoleManager {
  Tappable(SemanticsObject semanticsObject) : super(Role.tappable, semanticsObject) {
    _clickListener = createDomEventListener((DomEvent click) {
      final bool shouldDeduplicateClickEvent = PointerBinding.instance!.clickDebouncer.onClick(click);

      if (!_isListening || shouldDeduplicateClickEvent) {
        return;
      }

      EnginePlatformDispatcher.instance.invokeOnSemanticsAction(
          semanticsObject.id, ui.SemanticsAction.tap, null);
    });
    semanticsObject.element.addEventListener('click', _clickListener);
  }

  DomEventListener? _clickListener;
  bool _isListening = false;

  @override
  void update() {
    final bool wasListening = _isListening;
    _isListening = semanticsObject.enabledState() != EnabledState.disabled && semanticsObject.isTappable;
    if (wasListening != _isListening) {
      _updateAttribute();
    }
  }

  void _updateAttribute() {
    if (_isListening) {
      semanticsObject.element.setAttribute('flt-tappable', '');
    } else {
      semanticsObject.element.removeAttribute('flt-tappable');
    }
  }

  @override
  void dispose() {
    semanticsObject.element.removeEventListener('click', _clickListener);
    _clickListener = null;
    super.dispose();
  }
}
