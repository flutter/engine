// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:html' as html;
import 'dart:js_util' as js_util;
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('PlatformDispatcher', () {
    test('responds to flutter/skia Skia.setResourceCacheMaxBytes', () async {
      const MethodCodec codec = JSONMethodCodec();
      final Completer<ByteData?> completer = Completer<ByteData?>();
      ui.PlatformDispatcher.instance.sendPlatformMessage(
        'flutter/skia',
        codec.encodeMethodCall(const MethodCall(
          'Skia.setResourceCacheMaxBytes',
          512 * 1000 * 1000,
        )),
        completer.complete,
      );

      final ByteData? response = await completer.future;
      expect(response, isNotNull);
      expect(
        codec.decodeEnvelope(response!),
        <bool>[true],
      );
    });

    test('responds to flutter/platform HapticFeedback.vibrate', () async {
      const MethodCodec codec = JSONMethodCodec();
      final Completer<ByteData?> completer = Completer<ByteData?>();
      ui.PlatformDispatcher.instance.sendPlatformMessage(
        'flutter/platform',
        codec.encodeMethodCall(const MethodCall(
          'HapticFeedback.vibrate',
        )),
        completer.complete,
      );

      final ByteData? response = await completer.future;
      expect(response, isNotNull);
      expect(
        codec.decodeEnvelope(response!),
        true,
      );
    });

    test('responds correctly to flutter/platform Clipboard.getData failure',
        () async {
      // Patch browser so that clipboard api is not available.
      final Object? originalClipboard =
          js_util.getProperty<Object?>(html.window.navigator, 'clipboard');
      js_util.setProperty(html.window.navigator, 'clipboard', null);
      const MethodCodec codec = JSONMethodCodec();
      final Completer<ByteData?> completer = Completer<ByteData?>();
      ui.PlatformDispatcher.instance.sendPlatformMessage(
        'flutter/platform',
        codec.encodeMethodCall(const MethodCall(
          'Clipboard.getData',
        )),
        completer.complete,
      );
      final ByteData? response = await completer.future;
      if (response != null) {
        expect(
              () => codec.decodeEnvelope(response),
          throwsA(isA<PlatformException>()),
        );
      }
      js_util.setProperty(
          html.window.navigator, 'clipboard', originalClipboard);
    });

    test('can find text scale factor', () async {
      const double deltaTolerance = 1e-5;

      final html.Element root = html.document.documentElement!;
      final String oldFontSize = root.style.fontSize;

      addTearDown(() {
        root.style.fontSize = oldFontSize;
      });

      root.style.fontSize = '16px';
      expect(findBrowserTextScaleFactor(), 1.0);

      root.style.fontSize = '20px';
      expect(findBrowserTextScaleFactor(), 1.25);

      root.style.fontSize = '24px';
      expect(findBrowserTextScaleFactor(), 1.5);

      root.style.fontSize = '14.4px';
      expect(findBrowserTextScaleFactor(), closeTo(0.9, deltaTolerance));

      root.style.fontSize = '12.8px';
      expect(findBrowserTextScaleFactor(), closeTo(0.8, deltaTolerance));

      root.style.fontSize = null;
      expect(findBrowserTextScaleFactor(), 1.0);
    });

    test(
        'calls onTextScaleFactorChanged when the <html> element\'s font-size changes',
        () async {
      final html.Element root = html.document.documentElement!;
      final String oldFontSize = root.style.fontSize;
      final String oldBackgroundColor = root.style.backgroundColor;
      final String oldContentEditable = root.contentEditable;

      addTearDown(() {
        root.style.fontSize = oldFontSize;
        root.style.backgroundColor = oldBackgroundColor;
        root.contentEditable = oldContentEditable;
      });

      root.style.fontSize = '16px';
      root.style.backgroundColor = 'white';
      root.contentEditable = 'false';

      bool callsCallback = false;
      ui.PlatformDispatcher.instance.onTextScaleFactorChanged = () {
        callsCallback = true;
      };

      Future<void> waitUntilCalled() {
        final Completer completer = Completer();

        void check() {
          if (callsCallback == true) {
            completer.complete();
          } else {
            Timer(Duration.zero, check);
          }
        }

        check();
        return completer.future;
      }

      root.style.fontSize = '20px';
      await waitUntilCalled();
      expect(callsCallback, true);
      expect(ui.PlatformDispatcher.instance.textScaleFactor,
          findBrowserTextScaleFactor());

      callsCallback = false;

      root.style.fontSize = '16px';
      await waitUntilCalled();
      expect(callsCallback, true);
      expect(ui.PlatformDispatcher.instance.textScaleFactor,
          findBrowserTextScaleFactor());
    });
  });
}
