// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'dom.dart';
import 'platform_dispatcher.dart';
import 'services.dart';

final ByteData? _fontChangeMessage =
    const JSONMessageCodec().encodeMessage(
        <String, dynamic>{'type': 'fontsChange'});

// Font load callbacks will typically arrive in sequence, we want to prevent
// sendFontChangeMessage of causing multiple synchronous rebuilds.
// This flag ensures we properly schedule a single call to framework.
bool _fontChangeScheduled = false;

/// Notifies the framework that fonts have changed.
///
/// The framework is expected to rerender the UI using the new fonts, relaying
/// out text if necessary.
void sendFontChangeMessage() {
  if (!_fontChangeScheduled) {
    _fontChangeScheduled = true;
    // Batch updates into next animationframe.
    domWindow.requestAnimationFrame((_) {
      _fontChangeScheduled = false;
      EnginePlatformDispatcher.instance.invokeOnPlatformMessage(
        'flutter/system',
        _fontChangeMessage,
        (_) {},
      );
    });
  }
}
