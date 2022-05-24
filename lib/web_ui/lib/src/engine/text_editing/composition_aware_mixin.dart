import 'dart:html' as html;

import 'text_editing.dart';

typedef OnCompositionEndCallback = void Function(html.Event);
mixin CompositionAwareMixin {
  static const String kCompositionStart = 'compositionstart';
  static const String kCompositionUpdate = 'compositionupdate';
  static const String kCompositionEnd = 'compositionend';

  String? composingText;

  OnCompositionEndCallback? _onCompositionEndCallback;

  void addCompositionEventHandlers(html.HtmlElement domElement,
      {required OnCompositionEndCallback onCompositionEndCallback}) {
    domElement.addEventListener(kCompositionStart, _handleCompositionStart);
    domElement.addEventListener(kCompositionUpdate, _handleCompositionUpdate);
    domElement.addEventListener(kCompositionEnd, _handleCompositionEnd);

    _onCompositionEndCallback = onCompositionEndCallback;
  }

  void removeCompositionEventHandlers(html.HtmlElement domElement) {
    domElement.removeEventListener(kCompositionStart, _handleCompositionStart);
    domElement.removeEventListener(
        kCompositionUpdate, _handleCompositionUpdate);
    domElement.removeEventListener(kCompositionEnd, _handleCompositionEnd);

    _onCompositionEndCallback = null;
  }

  void _handleCompositionStart(html.Event event) {
    composingText = null;
  }

  void _handleCompositionUpdate(html.Event event) {
    if (event is html.CompositionEvent) {
      composingText = event.data;
    }
  }

  void _handleCompositionEnd(html.Event event) {
    assert(_onCompositionEndCallback != null,
        'cannot call composition update without having registered composition handlers');

    _onCompositionEndCallback!(event);
    composingText = null;
  }

  EditingState determineCompositionState(EditingState editingState) {
    if (editingState.baseOffset == null || editingState.baseOffset == -1) {
      return editingState;
    }

    if (composingText == null) {
      return editingState;
    }

    if (editingState.text == null) {
      return editingState;
    }

    final int composingBase = editingState.baseOffset! - composingText!.length;

    if (composingBase < 0) {
      return editingState;
    }

    return editingState.copyWith(
      composingBaseOffset: composingBase,
      composingExtentOffset: composingBase + composingText!.length,
    );
  }
}
