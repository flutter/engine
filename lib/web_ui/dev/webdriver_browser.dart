import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:image/image.dart';
import 'package:test_api/src/backend/runtime.dart';
import 'package:webdriver/io.dart' show createDriver, WebDriver, WebDriverSpec;

import 'browser.dart';

class WebDriverBrowserEnvironment extends BrowserEnvironment {
  late final Process _driverProcess;
  late final WebDriver _driver;

  @override
  Future<void> prepare() async {
    const int port = 4444;
    _driverProcess =
        await Process.start('safaridriver', <String>['-p', port.toString()]);

    _driverProcess.stderr
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((String error) {
      print('[Chromedriver] $error');
    });

    _driverProcess.stdout
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((String log) {
      print('[Chromedriver] $log');
    });
 
    await Future.delayed(const Duration(seconds: 1));

    final Uri driverUri = Uri(scheme: 'http', host: 'localhost', port: port);
    _driver =
        await createDriver(uri: driverUri, spec: WebDriverSpec.W3c, desired: <String, dynamic>{'browserName': 'safari'});
  }

  @override
  Future<void> cleanup() async {
    _driverProcess.kill();
  }

  @override
  String get packageTestConfigurationYamlFile => 'dart_test_safari.yaml';

  @override
  Runtime get packageTestRuntime => Runtime.safari;

  @override
  Browser launchBrowserInstance(Uri url, {bool debug = false}) {
    return WebDriverBrowser(_driver, url);
  }

  @override
  ScreenshotManager? getScreenshotManager() =>
      WebDriverScreenshotManager(_driver);
}

class WebDriverScreenshotManager extends ScreenshotManager {
  final WebDriver _driver;

  WebDriverScreenshotManager(this._driver);

  @override
  Future<Image> capture(Rectangle<num> region) async {
    final Image image = decodePng(await _driver.captureScreenshotAsList())!;
    return copyCrop(image, region.left.round(), region.top.round(),
        region.width.round(), region.height.round());
  }

  @override
  String get filenameSuffix => '';
}

class WebDriverBrowser extends Browser {
  @override
  String get name => 'safaridriver';

  final WebDriver _driver;
  final Uri _url;
  final Completer<void> _onExitCompleter = Completer<void>();

  WebDriverBrowser(this._driver, this._url) {
    _driver.get(_url);
  }

  @override
  Future<void> close() async {
    await (await _driver.window).close();
    if (!_onExitCompleter.isCompleted) {
      _onExitCompleter.complete();
    }
  }

  @override
  Future<void> get onExit => _onExitCompleter.future;
}
