// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'felt_config.dart';

class SuiteFilterResult {
  SuiteFilterResult.accepted();
  SuiteFilterResult.rejected(String reason) : rejectReason = reason;

  String? rejectReason;

  bool get isAccepted => rejectReason == null;
}

abstract class SuiteFilter {
  SuiteFilterResult filterSuite(TestSuite suite);
}

bool _macOSSupportsBrowser(Browser browser) {
  switch (browser) {
    case Browser.chrome:
    case Browser.firefox:
    case Browser.safari:
      return true;
    case Browser.edge:
     return false;
  }
}

bool _linuxSupportsBrowser(Browser browser) {
  switch (browser) {
    case Browser.chrome:
    case Browser.firefox:
      return true;
    case Browser.edge:
    case Browser.safari:
     return false;
  }
}

bool _windowsSupportsBrowser(Browser browser) {
  switch (browser) {
    case Browser.chrome:
    case Browser.edge:
      return true;
    case Browser.firefox:
    case Browser.safari:
     return false;
  }
}

typedef BrowserSupportCheck = bool Function(Browser browser);

BrowserSupportCheck get _platformBrowserSupportCheck {
  if (io.Platform.isLinux) {
    return _linuxSupportsBrowser;
  } else if (io.Platform.isMacOS) {
    return _macOSSupportsBrowser;
  } else if (io.Platform.isWindows) {
    return _windowsSupportsBrowser;
  } else {
    throw AssertionError('Unsupported OS: ${io.Platform.operatingSystem}');
  }
}

class PlatformSuiteFilter implements SuiteFilter {
  PlatformSuiteFilter() : _browserCheck = _platformBrowserSupportCheck;

  final BrowserSupportCheck _browserCheck;

  @override
  SuiteFilterResult filterSuite(TestSuite suite) {
    final Browser browser = suite.runConfig.browser;
    if (_browserCheck(browser)) {
      return SuiteFilterResult.accepted();
    } else {
      return SuiteFilterResult.rejected(
        'Current platform (${io.Platform.operatingSystem}) does not support browser $browser'
      );
    }
  }
}
