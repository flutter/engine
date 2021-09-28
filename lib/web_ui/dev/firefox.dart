// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert' show json;
import 'dart:io';

import 'package:path/path.dart' as path;
import 'package:pedantic/pedantic.dart';
import 'package:test_api/src/backend/runtime.dart';

import 'browser.dart';
import 'browser_lock.dart';
import 'common.dart';
import 'environment.dart';
import 'firefox_installer.dart';

/// Provides an environment for the desktop Firefox.
class FirefoxEnvironment implements BrowserEnvironment {
  @override
  Browser launchBrowserInstance(Uri url, {bool debug = false}) {
    return Firefox(url, debug: debug);
  }

  @override
  Runtime get packageTestRuntime => Runtime.firefox;

  @override
  Future<void> prepareEnvironment() async {
    // Firefox doesn't need any special prep.
  }

  @override
  String get packageTestConfigurationYamlFile => 'dart_test_firefox.yaml';

  @override
  ScreenshotManager? getScreenshotManager() => null;
}

/// Runs desktop Firefox.
///
/// Most of the communication with the browser is expected to happen via HTTP,
/// so this exposes a bare-bones API. The browser starts as soon as the class is
/// constructed, and is killed when [close] is called.
///
/// Any errors starting or running the process are reported through [onExit].
class Firefox extends Browser {
  @override
  final String name = 'Firefox';

  /// Starts a new instance of Firefox open to the given [url], which may be a
  /// [Uri] or a [String].
  factory Firefox(Uri url, {bool debug = false}) {
    return Firefox._(() async {
      final BrowserInstallation installation = await getOrInstallFirefox(
        browserLock.firefoxLock.version,
        infoLog: isCirrus ? stdout : DevNull(),
      );

      // Using a profile on opening will prevent popups related to profiles.
      const String _profile = '''
user_pref("browser.shell.checkDefaultBrowser", false);
user_pref("dom.disable_open_during_load", false);
user_pref("dom.max_script_run_time", 0);
''';

      final Directory temporaryProfileDirectory = Directory(
          path.join(environment.webUiDartToolDir.path, 'firefox_profile'));

      // A good source of various Firefox Command Line options:
      // https://developer.mozilla.org/en-US/docs/Mozilla/Command_Line_Options#Browser
      //
      if (temporaryProfileDirectory.existsSync()) {
        temporaryProfileDirectory.deleteSync(recursive: true);
      }
      temporaryProfileDirectory.createSync(recursive: true);

      File(path.join(temporaryProfileDirectory.path, 'prefs.js'))
          .writeAsStringSync(_profile);
      final bool isMac = Platform.isMacOS;
      final List<String> args = <String>[
        '--profile',
        temporaryProfileDirectory.path,
        if (!debug)
          '--headless',
        '-width $kMaxScreenshotWidth',
        '-height $kMaxScreenshotHeight',
        // On Mac Firefox uses the -- option prefix, while elsewhere it uses the - prefix.
        '${isMac ? '-' : ''}-new-window',
        '${isMac ? '-' : ''}-new-instance',
        'http://localhost:$kHealthCheckServerPort',
      ];

      final _HealthCheckServer healthCheckServer = await _HealthCheckServer.start(url);

      final Process process;
      final Object? healthCheckResult;
      try {
        process =
            await Process.start(installation.executable, args);

        // Wait for the browser to ping the health check. If the browser exits
        // before the health check, then something is wrong and we should
        // report the failure.
        healthCheckResult = await Future.any<Object?>(<Future<Object?>>[
          process.exitCode,
          healthCheckServer.whenHealthCheckPingReceived,
        ]);
      } catch (error) {
        rethrow;
      } finally {
        await healthCheckServer.close();
      }

      if (healthCheckResult is int) {
        throw Exception(
          'Firefox exited prematurely with exit code $healthCheckResult before reporting a health check.',
        );
      }

      unawaited(process.exitCode.then((_) {
        temporaryProfileDirectory.deleteSync(recursive: true);
      }));

      return process;
    });
  }

  Firefox._(Future<Process> Function() startBrowser)
      : super(startBrowser);
}

/// Host the initial page that redirects to the test page.
///
/// The browser's ability to load the initial page is used as a proof that the
/// browser has launched successfully, and therefore it's safe to load the test
/// page. When the initial page is requested by the browser, the
/// [whenHealthCheckPingReceived] resolves successfully.
class _HealthCheckServer {
  static Future<_HealthCheckServer> start(Uri url) async {
    final HttpServer server = await HttpServer.bind('localhost', kHealthCheckServerPort);
    return _HealthCheckServer._(url, server);
  }

  _HealthCheckServer._(Uri url, this._httpServer) {
    final Completer<void> completer = Completer<void>();
    whenHealthCheckPingReceived = completer.future;
    _httpServer.forEach((HttpRequest request) async {
      try {
        request.response.headers.set(HttpHeaders.contentTypeHeader, 'text/html');
        request.response.write('''
  <script>
    location = ${json.encode(url.toString())};
  </script>
        ''');

        // We need to flush because we won't keep the health check server
        // around. So we need to make sure the client has received the bytes
        // prior to shutting it down.
        await request.response.flush();
        await request.response.close();
        completer.complete();
      } catch (error, stackTrace) {
        completer.completeError(error, stackTrace);
      }
    });
  }

  final HttpServer _httpServer;

  /// Resolves when the health check server receives an HTTP request.
  late final Future<void> whenHealthCheckPingReceived;

  /// Shuts down the health check server.
  Future<void> close() async {
    await _httpServer.close(force: true);
  }
}
