import 'dart:html' as html;

import 'text_editing.dart';

mixin CompositionAwareMixin {
  String? composingText;

  void addCompositionEventHandlers(html.HtmlElement domElement) {
    domElement.addEventListener('compositionstart', _handleCompositionStart);
    domElement.addEventListener('compositionupdate', _handleCompositionUpdate);
    domElement.addEventListener('compositionend', _handleCompositionEnd);
  }

  void removeCompositionEventHandlers(html.HtmlElement domElement) {
    domElement.removeEventListener('compositionstart', _handleCompositionStart);
    domElement.removeEventListener('compositionupdate', _handleCompositionUpdate);
    domElement.removeEventListener('compositionend', _handleCompositionEnd);
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

    final int compositionExtent = editingState.baseOffset!;
    final int composingBase = compositionExtent - composingText!.length;

    if (composingBase < 0) {
      return editingState;
    }

    return editingState.copyWith(
      composingBaseOffset: composingBase,
      composingExtentOffset: composingBase + composingText!.length,
    );
  }
}
