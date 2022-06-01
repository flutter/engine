import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:image/image.dart';
import 'package:webdriver/io.dart' show createDriver, WebDriver;

import 'browser.dart';

abstract class WebDriverBrowserEnvironment extends BrowserEnvironment {
  late final Process _driverProcess;

  Future<Process> spawnDriverProcess();
  Uri get driverUri;

  @override
  Future<void> prepare() async {
    _driverProcess = await spawnDriverProcess();

    _driverProcess.stderr
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((String error) {
      print('[Webdriver] $error');
    });

    _driverProcess.stdout
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen((String log) {
      print('[Webdriver] $log');
    });
  }

  @override
  Future<void> cleanup() async {
    _driverProcess.kill();
  }

  @override
  Future<Browser> launchBrowserInstance(Uri url, {bool debug = false}) async {
    for(;;) {
      try {
        final WebDriver driver = await createDriver(
          uri: driverUri, desired: <String, dynamic>{'browserName': packageTestRuntime.identifier});
        return WebDriverBrowser(driver, url);
      } on SocketException {
        // Sometimes we may try to connect before the web driver port is ready. Retry.
        print('Failed to connect to webdriver process. Retrying in 100 ms');
        await Future<void>.delayed(const Duration(milliseconds: 100));
      } catch (exception) {
        print('Exception while creating webdriver: $exception');
        rethrow;
      }
    }
  }
}

class WebDriverBrowser extends Browser {
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

  @override
  bool get supportsScreenshots => true;

  @override
  Future<Image> captureScreenshot(Rectangle<num> region) async {
    final Image image = decodePng(await _driver.captureScreenshotAsList())!;
    return copyCrop(image, region.left.round(), region.top.round(),
        region.width.round(), region.height.round());
  }
}
