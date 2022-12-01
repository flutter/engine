// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import '../../engine.dart' show registerHotRestartListener;
import '../dom.dart';
import '../services.dart';
import '../util.dart';

/// Determines the assertiveness level of the accessibility announcement.
///
/// It is used to set the priority with which assistive technology should treat announcements.
///
/// The order of this enum must match the order of the values in semantics_event.dart in framework.
enum Assertiveness {
  polite,
  assertive,
}

/// Singleton for accessing accessibility announcements from the platform.
AccessibilityAnnouncements get accessibilityAnnouncements {
  assert(
    _accessibilityAnnouncements != null,
    'AccessibilityAnnouncements not initialized. Call initializeAccessibilityAnnouncements() to innitialize it.',
  );
  return _accessibilityAnnouncements!;
}
AccessibilityAnnouncements? _accessibilityAnnouncements;

void initializeAccessibilityAnnouncements() {
  assert(
    _accessibilityAnnouncements == null,
    'AccessibilityAnnouncements is already initialized. This is likely a bug in '
    'Flutter Web engine initialization. Please file an issue at '
    'https://github.com/flutter/flutter/issues/new/choose',
  );
  _accessibilityAnnouncements = AccessibilityAnnouncements();
  registerHotRestartListener(() {
    accessibilityAnnouncements.dispose();
  });
}

/// Attaches accessibility announcements coming from the 'flutter/accessibility'
/// channel as temporary elements to the DOM.
class AccessibilityAnnouncements {
  factory AccessibilityAnnouncements() {
    final DomHTMLElement politeElement = _createElement(Assertiveness.polite);
    final DomHTMLElement assertiveElement = _createElement(Assertiveness.assertive);
    domDocument.body!.append(politeElement);
    domDocument.body!.append(assertiveElement);
    return AccessibilityAnnouncements._(politeElement, assertiveElement);
  }

  AccessibilityAnnouncements._(this._politeElement, this._assertiveElement);

  /// A live region element with `aria-live` set to "polite", used to announce
  /// accouncements politely.
  final DomHTMLElement _politeElement;

  /// A live region element with `aria-live` set to "assertive", used to announce
  /// accouncements assertively.
  final DomHTMLElement _assertiveElement;

  DomHTMLElement ariaLiveElementFor(Assertiveness assertiveness) {
    assert(!_isDisposed);
    switch (assertiveness) {
      case Assertiveness.polite: return _politeElement;
      case Assertiveness.assertive: return _assertiveElement;
    }
  }

  bool _isDisposed = false;

  void dispose() {
    assert(!_isDisposed);
    _isDisposed = true;
    _politeElement.remove();
    _assertiveElement.remove();
    _accessibilityAnnouncements = null;
  }

  /// Decodes the message coming from the 'flutter/accessibility' channel.
  void handleMessage(StandardMessageCodec codec, ByteData? data) {
    assert(!_isDisposed);
    final Map<dynamic, dynamic> inputMap =
        codec.decodeMessage(data) as Map<dynamic, dynamic>;
    final Map<dynamic, dynamic> dataMap = inputMap.readDynamicJson('data');
    final String? message = dataMap.tryString('message');
    if (message != null && message.isNotEmpty) {
      /// The default value for politeness is `polite`.
      final int assertivenessIndex = dataMap.tryInt('assertiveness') ?? 0;
      final Assertiveness assertiveness = Assertiveness.values[assertivenessIndex];
      final DomHTMLElement liveRegion = ariaLiveElementFor(assertiveness);

      // Create an additional child so that if the text of the announcement is
      // the same as before, it is announced again.
      final DomHTMLDivElement textContainer = createDomHTMLDivElement();
      textContainer.text = message;
      liveRegion.clearChildren();
      liveRegion.appendChild(textContainer);
    }
  }

  static DomHTMLLabelElement _createElement(Assertiveness assertiveness) {
    final String ariaLiveValue = (assertiveness == Assertiveness.assertive) ? 'assertive' : 'polite';
    final DomHTMLLabelElement liveRegion = createDomHTMLLabelElement();
    liveRegion.setAttribute('id', 'ftl-announcement-$ariaLiveValue');
    liveRegion.style
      ..position = 'fixed'
      ..overflow = 'hidden'
      ..transform = 'translate(-99999px, -99999px)'
      ..width = '1px'
      ..height = '1px';
    liveRegion.setAttribute('aria-live', ariaLiveValue);
    return liveRegion;
  }
}
