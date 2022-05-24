import 'dart:html' as html;

import 'text_editing.dart';

mixin CompositionAwareMixin {
  static const String kCompositionStart = 'compositionstart';
  static const String kCompositionUpdate = 'compositionupdate';
  static const String kCompositionEnd = 'compositionend';

  late final html.EventListener _compositionStartListener = _handleCompositionStart;
  late final html.EventListener _compositionUpdateListener = _handleCompositionUpdate;
  late final html.EventListener _compositionEndListener = _handleCompositionEnd;

  String? composingText;

  void addCompositionEventHandlers(html.HtmlElement domElement) {
    domElement.addEventListener(kCompositionStart, _compositionStartListener);
    domElement.addEventListener(kCompositionUpdate, _compositionUpdateListener);
    domElement.addEventListener(kCompositionEnd, _compositionEndListener);
  }

  void removeCompositionEventHandlers(html.HtmlElement domElement) {
    domElement.removeEventListener(kCompositionStart, _compositionStartListener);
    domElement.removeEventListener(kCompositionUpdate, _compositionUpdateListener);
    domElement.removeEventListener(kCompositionEnd, _compositionEndListener);
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
    composingText = null;
  }

  EditingState determineCompositionState(EditingState editingState) {
    if (editingState.baseOffset == null) {
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
