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

class MockWithCompositionAwareMixin with CompositionAwareMixin {}

html.HtmlElement get inputElement {
  return defaultTextEditingRoot.querySelectorAll('input').first
      as html.HtmlElement;
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
      onChange: trackEditingState,
      onAction: trackInputAction,
    );
  });

  tearDown(() {
    editingStrategy.disable();
  });

  group('$CompositionAwareMixin', () {
    group('composition end', () {
      test('should reset composing text on handle composition end', () {
        final MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(inputElement,
            onCompositionEndCallback: (_) {});

        inputElement
            .dispatchEvent(html.Event(CompositionAwareMixin.kCompositionEnd));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });

      test('should call onCompositionEndCallback', () {
        const String fakeEventText = 'IAmComposingThis';
        final MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        final Completer<html.Event> didCallOnCompositionEndCallback =
            Completer<html.Event>();
        mockWithCompositionAwareMixin.addCompositionEventHandlers(inputElement,
            onCompositionEndCallback: didCallOnCompositionEndCallback.complete);

        inputElement.dispatchEvent(html.CompositionEvent(
            CompositionAwareMixin.kCompositionEnd,
            data: fakeEventText));

        expect(didCallOnCompositionEndCallback.isCompleted, isTrue);
      });
    });

    group('composition start', () {
      test('should reset composing text on handle composition start', () {
        final MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(inputElement,
            onCompositionEndCallback: (_) {});

        inputElement
            .dispatchEvent(html.Event(CompositionAwareMixin.kCompositionStart));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });
    });

    group('composition update', () {
      test('should set composing text to event composing text', () {
        const String fakeEventText = 'IAmComposingThis';
        final MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(inputElement,
            onCompositionEndCallback: (_) {});

        inputElement.dispatchEvent(html.CompositionEvent(
            CompositionAwareMixin.kCompositionUpdate,
            data: fakeEventText));

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

        final MockWithCompositionAwareMixin mockWithCompositionAwareMixin =
            MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = composingText;

        const int expectedComposingBase = baseOffset - composingText.length;

        expect(
            mockWithCompositionAwareMixin
                .determineCompositionState(editingState),
            editingState.copyWith(
                composingBaseOffset: expectedComposingBase,
                composingExtentOffset: expectedComposingBase + composingText.length));
      });
    });
  });
}
