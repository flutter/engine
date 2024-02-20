// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:js_interop';

import 'package:js/js_util.dart' as js_util;
import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart' as engine;
import 'package:ui/src/engine/browser_detection.dart';
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;

@JS('_flutter')
external set _loader(JSAny? loader);
set loader(Object? l) => _loader = l?.toJSAnyShallow;

@JS('_flutter.loader.didCreateEngineInitializer')
external set didCreateEngineInitializer(JSFunction? callback);

void main() {
  // Prepare _flutter.loader.didCreateEngineInitializer, so it's ready in the page ASAP.
  loader = js_util.jsify(<String, Object>{
    'loader': <String, Object>{
      'didCreateEngineInitializer': () { print('not mocked'); }.toJS,
    },
  });
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  test('bootstrapEngine calls _flutter.loader.didCreateEngineInitializer callback', () async {
    JSAny? engineInitializer;

    void didCreateEngineInitializerMock(JSAny? obj) {
      print('obj: $obj');
      engineInitializer = obj;
    }

    // Prepare the DOM for: _flutter.loader.didCreateEngineInitializer
    didCreateEngineInitializer = didCreateEngineInitializerMock.toJS;

    // Reset the engine
    engine.debugResetEngineInitializationState();

    await ui_web.bootstrapEngine(
      registerPlugins: () {},
      runApp: () {},
    );

    // Check that the object we captured is actually a loader
    expect(engineInitializer, isNotNull);
    expect(js_util.hasProperty(engineInitializer!, 'initializeEngine'), isTrue, reason: 'Missing FlutterEngineInitializer method: initializeEngine.');
    expect(js_util.hasProperty(engineInitializer!, 'autoStart'), isTrue, reason: 'Missing FlutterEngineInitializer method: autoStart.');
  });

  test('bootstrapEngine does auto-start when _flutter.loader.didCreateEngineInitializer does not exist', () async {
    loader = null;

    bool pluginsRegistered = false;
    bool appRan = false;
    void registerPluginsMock() {
      pluginsRegistered = true;
    }
    void runAppMock() {
      appRan = true;
    }

    // Reset the engine
    engine.debugResetEngineInitializationState();

    await ui_web.bootstrapEngine(
      registerPlugins: registerPluginsMock,
      runApp: runAppMock,
    );

    // Check that the object we captured is actually a loader
    expect(pluginsRegistered, isTrue, reason: 'Plugins should be immediately registered in autoStart mode.');
    expect(appRan, isTrue, reason: 'App should run immediately in autoStart mode');
  });

  const Map<String, String> crawlerUserAgents = <String, String>{
    'Yahoo': 'Mozilla/5.0 (compatible; Yahoo! Slurp; http://help.yahoo.com/help/us/ysearch/slurp)',
    'Bing': 'Mozilla/5.0 (compatible; bingbot/2.0; +http://www.bing.com/bingbot.htm)',
    'Yandex': 'Mozilla/5.0 (compatible; YandexBot/3.0; +http://yandex.com/bots)',
    'Google': 'Mozilla/5.0 (Linux; Android 6.0.1; Nexus 5X Build/MMB29P) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/W.X.Y.Z Mobile Safari/537.36 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)',
    'Google Storebot': 'Mozilla/5.0 (X11; Linux x86_64; Storebot-Google/1.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.88 Safari/537.36',
    'Google AdsBot': 'Mozilla/5.0 (Linux; Android 6.0.1; Nexus 5X Build/MMB29P) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/W.X.Y.Z Mobile Safari/537.36 (compatible; AdsBot-Google-Mobile; +http://www.google.com/mobile/adsbot.html)',
    'Google AdSense': 'Mediapartners-Google',
    'DuckDuckGo': 'DuckDuckBot-Https/1.1; (+https://duckduckgo.com/duckduckbot)',
    'Baidu': 'Mozilla/5.0 (compatible; Baiduspider/2.0; +http://www.baidu.com/search/spider.html)',
  };

  const Map<String, String> nonCrawlerUserAgents = <String, String>{
    'Chrome for Android': 'Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Mobile Safari/537.36',
    'Chrome for Windows': 'Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36',
    'Chrome for macOS': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36',
    'Firefox for Windows': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0',
    'Firefox for macOS': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:101.0) Gecko/20100101 Firefox/101.0',
    'Edge for Windows': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0',
    'Safari for macOS': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.3.1 Safari/605.1.15',
    'Safari for iOS': 'Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.0 Mobile/15E148 Safari/604.1',
  };

  test('detectCrawler detects a web crawler', () {
    String testUserAgentString = '';

    getUserAgent = () => testUserAgentString;
    addTearDown(() {
      getUserAgent = defaultGetUserAgent;
    });

    for (final MapEntry<String, String> testAgent in crawlerUserAgents.entries) {
      testUserAgentString = testAgent.value;
      expect(
        reason: '${testAgent.key} is a crawler',
        ui_web.detectCrawler(),
        isTrue,
      );
    }
  });

  test('detectCrawler does not detect a web crawler', () {
    String testUserAgentString = '';

    getUserAgent = () => testUserAgentString;
    addTearDown(() {
      getUserAgent = defaultGetUserAgent;
    });

    for (final MapEntry<String, String> testAgent in nonCrawlerUserAgents.entries) {
      testUserAgentString = testAgent.value;
      expect(
        reason: '${testAgent.key} is not a crawler',
        ui_web.detectCrawler(),
        isFalse,
      );
    }
  });

  // We cannot test anymore, because by now the engine has registered some stuff that can't be rewound back.
  // Like the `ext.flutter.disassemble` developer extension.
}
