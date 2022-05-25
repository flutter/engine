// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:html' as html;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

import 'package:ui/src/engine/initialization.dart';
import 'package:ui/src/engine/text_editing/composition_aware_mixin.dart';
import 'package:ui/src/engine/text_editing/input_type.dart';
import 'package:ui/src/engine/text_editing/text_editing.dart';

import 'text_editing_test.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

class _MockWithCompositionAwareMixin with CompositionAwareMixin {
  // These variables should be equal to their counterparts in CompositionAwareMixin.
  // Seperate so the counterparts in CompositionAwareMixin can be private.
  static const String _kCompositionUpdate = 'compositionupdate';
  static const String _kCompositionStart = 'compositionstart';
  static const String _kCompositionEnd = 'compositionend';
}

html.InputElement get _inputElement {
  return defaultTextEditingRoot.querySelectorAll('input').first as html.InputElement;
}

Future<void> testMain() async {
  await initializeEngine();

  final GloballyPositionedTextEditingStrategy editingStrategy =
      GloballyPositionedTextEditingStrategy(HybridTextEditing()
        ..configuration = InputConfiguration(
          inputType: EngineInputType.text,
        ));
  editingStrategy.owner.debugTextEditingStrategyOverride = editingStrategy;
  const String fakeComposingText = 'ImComposingText';

  setUp(() {
    editingStrategy.enable(
      singlelineConfig,
      onChange: (_, __) {},
      onAction: (_) {},
    );
  });

  tearDown(() {
    editingStrategy.disable();
  });

  group('$CompositionAwareMixin', () {
    group('composition end', () {
      test('should reset composing text on handle composition end', () {
        final _MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(html.Event(_MockWithCompositionAwareMixin._kCompositionEnd));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });
    });

    group('composition start', () {
      test('should reset composing text on handle composition start', () {
        final _MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(html.Event(_MockWithCompositionAwareMixin._kCompositionStart));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });
    });

    group('composition update', () {
      test('should set composing text to event composing text', () {
        const String fakeEventText = 'IAmComposingThis';
        final _MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(html.CompositionEvent(_MockWithCompositionAwareMixin._kCompositionUpdate, data: fakeEventText));

        expect(mockWithCompositionAwareMixin.composingText, fakeEventText);
      });
    });

    group('determine composition state', () {
      test('should return new composition state if valid new composition', () {
        const int baseOffset = 100;
        const String composingText = 'composeMe';

        final EditingState editingState = EditingState(
          extentOffset: baseOffset,
          text: 'testing',
          baseOffset: baseOffset,
        );

        final _MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = composingText;

        const int expectedComposingBase = baseOffset - composingText.length;

        expect(
            mockWithCompositionAwareMixin.determineCompositionState(editingState),
            editingState.copyWith(
                composingBaseOffset: expectedComposingBase,
                composingExtentOffset: expectedComposingBase + composingText.length
              )
          );
      });
    });
  });

  group('composing range', () {
    test('should be [0, compostionStrLength] on new composition', () {
      const String composingText = 'hi';

      _inputElement.dispatchEvent(html.CompositionEvent(_MockWithCompositionAwareMixin._kCompositionUpdate, data: composingText));

      // Set the selection text.
      _inputElement.value = composingText;
      _inputElement.dispatchEvent(html.Event.eventType('Event', 'input'));

      expect(
          editingStrategy.lastEditingState,
          isA<EditingState>()
              .having((EditingState editingState) => editingState.composingBaseOffset,
                  'composingBaseOffset', 0)
              .having((EditingState editingState) => editingState.composingExtentOffset,
                  'composingExtentOffset', composingText.length)
          );
    });

    test(
        'should be [beforeComposingText - composingText, compostionStrLength] on composition in the middle of text',
        () {
      const String composingText = 'hi';
      const String beforeComposingText = 'beforeComposingText';
      const String afterComposingText = 'afterComposingText';

      // Type in the text box, then move cursor to the middle.
      _inputElement.value = '$beforeComposingText$afterComposingText';
      _inputElement.setSelectionRange(beforeComposingText.length, beforeComposingText.length);

      _inputElement.dispatchEvent(
          html.CompositionEvent(_MockWithCompositionAwareMixin._kCompositionUpdate, data: composingText));

      // Flush editing state (since we did not compositionend).
      _inputElement.dispatchEvent(html.Event.eventType('Event', 'input'));

      expect(
          editingStrategy.lastEditingState,
          isA<EditingState>()
              .having((EditingState editingState) => editingState.composingBaseOffset!,
                  'composingBaseOffset', beforeComposingText.length - composingText.length)
              .having((EditingState editingState) => editingState.composingExtentOffset,
                  'composingExtentOffset', beforeComposingText.length)
        );
    });
  });
}
