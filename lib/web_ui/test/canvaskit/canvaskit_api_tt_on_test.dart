// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:js/js_util.dart' as js_util;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import '../matchers.dart';
import 'canvaskit_api_test.dart';

// These tests need to happen in a separate file, because a Content Security
// Policy cannot be relaxed once set, only made more strict.
void main() {
  enableTrustedTypes();
  internalBootstrapBrowserTest(() => () {
    // Run all standard canvaskit tests, with TT on...
    testMain();

    test('rejects wrong canvaskit.js URL', () async {
      expect(() {
        createTrustedScriptUrl('https://www.unpkg.com/soemthing/not-canvaskit.js');
      }, throwsAssertionError);
    });
  });
}

/// Enables Trusted Types by setting the appropriate meta tag in the DOM:
/// <meta http-equiv="Content-Security-Policy" content="require-trusted-types-for 'script'">
void enableTrustedTypes() {
  print('Enabling TrustedTypes in browser window...');
  final DomHTMLMetaElement enableTTMeta = createDomHTMLMetaElement()
    ..setAttribute('http-equiv', 'Content-Security-Policy')
    ..content = "require-trusted-types-for 'script'";
  domDocument.head!.append(enableTTMeta);
}
