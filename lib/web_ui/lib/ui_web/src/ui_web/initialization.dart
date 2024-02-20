// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

/// Bootstraps the Flutter Web engine and app.
///
/// If the app uses plugins, then the [registerPlugins] callback can be provided
/// to register those plugins. This is done typically by calling
/// `registerPlugins` from the auto-generated `web_plugin_registrant.dart` file.
///
/// The [runApp] callback is invoked to run the app after the engine is fully
/// initialized.
///
/// For more information, see what the `flutter_tools` does in the entrypoint
/// that it generates around the app's main method:
///
/// * https://github.com/flutter/flutter/blob/95be76ab7e3dca2def54454313e97f94f4ac4582/packages/flutter_tools/lib/src/web/file_generators/main_dart.dart#L14-L43
///
/// By default, engine initialization and app startup occur immediately and back
/// to back. They can be programmatically controlled by setting
/// `FlutterLoader.didCreateEngineInitializer`. For more information, see how
/// `flutter.js` does it:
///
/// * https://github.com/flutter/flutter/blob/95be76ab7e3dca2def54454313e97f94f4ac4582/packages/flutter_tools/lib/src/web/file_generators/js/flutter.js
Future<void> bootstrapEngine({
  ui.VoidCallback? registerPlugins,
  ui.VoidCallback? runApp,
}) async {
  // Create the object that knows how to bootstrap an app from JS and Dart.
  final AppBootstrap bootstrap = AppBootstrap(
    initializeEngine: ([JsFlutterConfiguration? configuration]) async {
      await initializeEngineServices(jsConfiguration: configuration);
    }, runApp: () async {
      if (registerPlugins != null) {
        registerPlugins();
      }
      await initializeEngineUi();
      if (runApp != null) {
        runApp();
      }
    },
  );

  final FlutterLoader? loader = flutter?.loader;
  if (loader == null || loader.isAutoStart) {
    // The user does not want control of the app, bootstrap immediately.
    domWindow.console.debug('Flutter Web Bootstrap: Auto.');
    await bootstrap.autoStart();
  } else {
    // Yield control of the bootstrap procedure to the user.
    domWindow.console.debug('Flutter Web Bootstrap: Programmatic.');
    loader.didCreateEngineInitializer(bootstrap.prepareEngineInitializer());
  }
}

/// The signature of the [detectCrawler] function.
typedef CrawlerDetector = bool Function();

/// Determines whether the current user agent is a web crawler, such as a search
/// engine.
///
/// The web engine calls this method once during initialization, and then it no
/// longer calls this method. Overriding this function will have no effect after
/// it has been called.
///
/// Many crawlers ignore ARIA attributes as a source of app semantics. Therefore,
/// if a crawler is detected, the web engine outputs semantic labels as text in
/// a DOM `<span>` element, which crawlers know how to read.
///
/// This function can be overriden by the app, by setting it to a different
/// function. This can be useful when the app needs to support a crawler that's
/// not known to the Flutter SDK.
///
/// The default implementation is [defaultDetectCrawler].
CrawlerDetector detectCrawler = defaultDetectCrawler;

// The list of known crawler user agent patterns. It should capture all of the
// most important crawlers, and so it may change over time. However, it is not
// meant to be comprehensive. Instead, the user can override `detectCrawler`
// with a custom implementation. Developers can also share packages on pub.dev
// that have sophisticated ways of detecting crawlers.
final List<RegExp> _crawlerRegexes = <RegExp>[
  // Covers the search engine and various specialized crawlers: https://developers.google.com/search/docs/crawling-indexing/overview-google-crawlers
  RegExp(r'googlebot'),
  RegExp(r'\-google'),

  // https://www.bing.com/webmasters/help/which-crawlers-does-bing-use-8c184ec0
  RegExp(r'bingbot'),

  // https://help.yahoo.com/kb/SLN22600.html?guccounter=1&guce_referrer=aHR0cHM6Ly93d3cuZ29vZ2xlLmNvbS8&guce_referrer_sig=AQAAAHYCkkIs7cZnhukSWyF2fm5fvmF4O0hHyVAfflpLGgpkUyKE_oPtyuTbPEnHEw9hJfgfyD5mnYtUdWngKIq-aUwiRz_Kz7G_5I7jbyvZyiXyObYZlltmEazI-97JcSOG-AbDLdNMATVgEW23LgFVADYtPkr8VcsKfMem8B0eZBWA
  RegExp(r'slurp'),

  // https://duckduckgo.com/duckduckbot
  RegExp(r'duckduckbot'),

  // http://www.baidu.com/search/spider.html
  RegExp(r'spider'),

  // https://yandex.com/support/webmaster/robot-workings/user-agent.html
  RegExp(r'yandexbot'),

  // yjbanov: at the time of writing this code I didn't know which crawlers used
  //          the "crawl" sub-string, but multiple sources recommended including
  //          it, e.g.:
  //
  //          https://stackoverflow.com/questions/20084513/detect-search-crawlers-via-javascript
  RegExp(r'crawl'),
];

/// The default implementation of [detectCrawler].
///
/// This implementation is likely not complete. The web is too big for the
/// Flutter SDK to be able to detect them all. If a particular crawler is
/// important but is missed by this implementation, [detectCrawler] can be
/// overridden.
bool defaultDetectCrawler() {
  final String userAgent = getUserAgent().toLowerCase();
  return _crawlerRegexes.any((RegExp regexp) => regexp.hasMatch(userAgent));
}
