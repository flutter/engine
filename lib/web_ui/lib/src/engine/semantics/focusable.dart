// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../dom.dart';
import '../platform_dispatcher.dart';
import '../safe_browser_api.dart';
import '../util.dart';
import 'semantics.dart';

/// Supplies generic accessibility focus features to semantics nodes that have
/// [ui.SemanticsFlag.isFocusable] set.
///
/// Assumes that the element being focused on is [SemanticsObject.element]. Role
/// managers with special needs can implement custom focus management and
/// exclude this role manager.
///
/// `"tab-index=0"` is used because `<flt-semantics>` is not intrinsically
/// focusable. Examples of intrinsically focusable elements include:
///
///   * <button>
///   * <input> (of any type)
///   * <a>
///   * <textarea>
///
/// See also:
///
///   * https://developer.mozilla.org/en-US/docs/Web/Accessibility/Keyboard-navigable_JavaScript_widgets
class Focusable extends RoleManager {
  Focusable(SemanticsObject semanticsObject)
      : _focusManager = AccessibilityFocusManager(semanticsObject.owner),
        super(Role.focusable, semanticsObject);

  final AccessibilityFocusManager _focusManager;

  @override
  void update() {
    if (semanticsObject.isFocusable) {
      if (!_focusManager.isManaging) {
        _focusManager.manage(semanticsObject.id, semanticsObject.element);
      }
      _focusManager.changeFocus(semanticsObject.hasFocus && (!semanticsObject.hasEnabledState || semanticsObject.isEnabled));
    } else {
      _focusManager.stopManaging();
    }
  }

  @override
  void dispose() {
    super.dispose();
    _focusManager.stopManaging();
  }
}

/// Objects associated with the element whose focus is being managed.
typedef _FocusTarget = ({
  /// [SemanticsObject.id] of the semantics node being managed.
  int semanticsNodeId,

  /// The element whose focus is being managed.
  DomElement element,

  /// The listener for the "focus" DOM event.
  DomEventListener domFocusListener,

  /// The listener for the "blur" DOM event.
  DomEventListener domBlurListener,
});

/// Implements accessibility focus management for arbitrary elements.
///
/// Unlike [Focusable], which implements focus features on [SemanticsObject]s
/// whose [SemanticsObject.element] is directly focusable, this class can help
/// implementing focus features on custom elements. For example, [Incrementable]
/// uses a custom `<input>` tag internally while its root-level element is not
/// focusable. However, it can still use this class to manage the focus of the
/// internal element.
class AccessibilityFocusManager {
  /// Creates a focus manager tied to a specific [EngineSemanticsOwner].
  AccessibilityFocusManager(this._owner);

  final EngineSemanticsOwner _owner;

  _FocusTarget? _target;

  /// Whether this focus manager is managing a focusable target.
  bool get isManaging => _target != null;

  /// Starts managing the focus of the given [element].
  ///
  /// The "focus" and "blur" DOM events are forwarded to the framework-side
  /// semantics node with ID [semanticsNodeId] as [ui.SemanticsAction]s.
  ///
  /// If this manage was already managing a different element, stops managing
  /// the old element and starts managing the new one.
  ///
  /// Calling this with the same element but a different [semanticsNodeId] will
  /// cause any future focus/blur events to be forwarded to the new ID.
  void manage(int semanticsNodeId, DomElement element) {
    if (identical(element, _target?.element)) {
      final _FocusTarget previousTarget = _target!;
      if (semanticsNodeId == previousTarget.semanticsNodeId) {
        return;
      }

      // No need to hook up new DOM listeners. The existing ones are good enough.
      _target = (
        semanticsNodeId: semanticsNodeId,
        element: previousTarget.element,
        domFocusListener: previousTarget.domFocusListener,
        domBlurListener: previousTarget.domBlurListener,
      );
      return;
    }

    if (_target != null) {
      // The element changed. Clear the old element before initializing the new one.
      stopManaging();
    }

    final _FocusTarget newTarget = (
      semanticsNodeId: semanticsNodeId,
      element: element,
      domFocusListener: createDomEventListener((_) => _onDidReceiveFocusEvent(true)),
      domBlurListener: createDomEventListener((_) => _onDidReceiveFocusEvent(false)),
    );
    _target = newTarget;

    element.tabIndex = 0;
    element.addEventListener('focus', newTarget.domFocusListener);
    element.addEventListener('blur', newTarget.domBlurListener);
  }

  /// Stops managing the focus of the current element, if any.
  ///
  /// Does not remove the `tabIndex` attribute. This is because a focused
  /// element cannot be removed immediately, but after the focus is transferred
  /// elsewhere (see [SemanticsObject.leaveTombstone]).
  ///
  /// Does not blur the element because the tombstone must remain focused.
  void stopManaging() {
    final _FocusTarget? target = _target;
    _target = null;
    _lastSetValue = null;

    if (target == null) {
      /// Nothing is being managed. Just return.
      return;
    }

    target.element.removeEventListener('focus', target.domFocusListener);
    target.element.removeEventListener('blur', target.domBlurListener);
  }

  void _onDidReceiveFocusEvent(bool didReceiveFocus) {
    final _FocusTarget? target = _target;

    if (target == null) {
      // DOM events can be asynchronous. By the time the event reaches here, the
      // focus manager may have been disposed of.
      return;
    }

    EnginePlatformDispatcher.instance.invokeOnSemanticsAction(
      target.semanticsNodeId,
      didReceiveFocus
        ? ui.SemanticsAction.didGainAccessibilityFocus
        : ui.SemanticsAction.didLoseAccessibilityFocus,
      null,
    );
  }

  bool? _lastSetValue;

  /// Requests focus or blur on the DOM element.
  void changeFocus(bool value) {
    // Only request focus/blur if the framework has not already asked for it. It
    // is possible that the DOM element no longer has focus but the framework
    // still thinks that it should have it and requests it again. If the DOM
    // focus is requested eagerly it could lead to scrolling bugs, such as:
    //
    // - The user wishes to scroll down by moving screen reader focus ring to an
    //   element below the bottom edge of the scroll viewport.
    // - Screen reader shifts DOM focus to that element.
    // - The previous element receives `didLoseAccessibilityFocus`.
    // - The next element recieves `didGainAccessibilityFocus`.
    // - The widgets corresponding to the aforementioned elements do not support
    //   `didLoseAccessibilityFocus`/`didGainAccessibilityFocus` and blissfully
    //   ignore them.
    // - A semantic update is received that still wants the previous element to
    //   be focused. But, if domElement.focus() is invoked, the browser will
    //   scroll back to that element, which is not what the user wanted.
    //
    // To counter this issue, if changeFocus(true) is called for the first time,
    // the DOM focus is shifted to the managed element. If changeFocus(true) is
    // called again without calling changeFocus(false) prior to it, the DOM
    // focus is not requested.
    //
    // This trick does not work in all situations. When a node is removed and
    // later readded as a result of lazy-rendering, it would cause the focus
    // manager to request focus again. More details in this issue (the web is
    // not the only one affected): https://github.com/flutter/flutter/issues/130844
    if (value == _lastSetValue) {
      return;
    }
    _lastSetValue = value;

    final _FocusTarget? target = _target;

    if (target == null) {
      // Nothing is being managed right now.
      assert(() {
        printWarning(
          'Cannot change focus to $value. No element is being managed by this '
          'AccessibilityFocusManager.'
        );
        return true;
      }());
      return;
    }

    // If attempting to shift focus to this DOM node but the node is already
    // focused, do nothing. Calling `DomElement.focus()` causes the browser to
    // scroll to the element if the element is outside the scroll viewport, and
    // scrolling back to the element would cause the framework to fail to scroll
    // where the user wants.
    if (value) {
      final DomElement? activeElement = getActiveElementInNearestShadowScope(target.element);
      if (target.element == activeElement) {
        return;
      }
    }

    // Delay the focus request until the final DOM structure is established
    // because the element may not yet be attached to the DOM, or it may be
    // reparented and lose focus again.
    _owner.addOneTimePostUpdateCallback(() {
      if (_target != target) {
        // The element may have been swapped or the manager may have been disposed
        // of between the focus change request and the post update callback
        // invocation. So check again that the element is still the same and is
        // not null.
        return;
      }

      if (value) {
        target.element.focus();
      } else {
        target.element.blur();
      }
    });
  }
}
