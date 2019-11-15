// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test_api/src/backend/runtime.dart';

import 'browser.dart';
import 'chrome.dart';
import 'chrome_installer.dart';
import 'common.dart';
import 'firefox.dart';
import 'firefox_installer.dart'; // ignore: implementation_imports

/// Class that has mappings and utility methods for the browsers tests are
/// supported.
class SupportedBrowsers {
  final List<BrowserArgParser> argParsers =
      List.of([ChromeArgParser(), FirefoxArgParser()]);

  final List<String> supportedBrowserNames = ['chrome', 'firefox'];

  final Map<String, Runtime> supportedBrowsersToRuntimeMap = {
    'chrome': Runtime.chrome,
    'firefox': Runtime.firefox
  };

  static final SupportedBrowsers _singletonInstance = SupportedBrowsers._();

  factory SupportedBrowsers() {
    return _singletonInstance;
  }

  SupportedBrowsers._();

  Browser getBrowser(Runtime runtime, Uri url, {bool debug = false}) {
    if (runtime ==  Runtime.chrome) {
      return Chrome(url, debug: debug);
    } else if (runtime == Runtime.firefox) {
      return Firefox(url, debug: debug);
    } else {
      throw new UnsupportedError('The browser type not supported in tests');
    }
  }
}
