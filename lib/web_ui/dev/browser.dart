// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:math' as math;

import 'package:image/image.dart';
import 'package:test_api/src/backend/runtime.dart';

/// Provides the environment for a specific web browser.
abstract class BrowserEnvironment {
  /// The [Runtime] used by `package:test` to identify this browser type.
  Runtime get packageTestRuntime;

  /// The name of the configuration YAML file used to configure `package:test`.
  ///
  /// The configuration file is expected to be a direct child of the `web_ui`
  /// directory.
  String get packageTestConfigurationYamlFile;

  /// Prepares the OS environment to run tests for this browser.
  ///
  /// This may include things like installing browsers, and starting web drivers,
  /// iOS Simulators, and/or Android emulators.
  ///
  /// Typically the browser environment is prepared once and supports multiple
  /// browser instances.
  Future<void> prepare();

  // Perform any necessary teardown steps
  Future<void> cleanup();

  /// Launches a browser instance.
  ///
  /// The browser will be directed to open the provided [url].
  ///
  /// If [debug] is true and the browser supports debugging, launches the
  /// browser in debug mode by pausing test execution after the code is loaded
  /// but before calling the `main()` function of the test, giving the
  /// developer a chance to set breakpoints.
  Browser launchBrowserInstance(Uri url, {bool debug = false});

  /// Returns the screenshot manager used by this browser.
  ///
  /// If the browser does not support screenshots, returns null.
  ScreenshotManager? getScreenshotManager();
}

/// An interface for running browser instances.
///
/// This is intentionally coarse-grained: browsers are controlled primary from
/// inside a single tab. Thus this interface only provides support for closing
/// the browser and seeing if it closes itself.
///
/// Any errors starting or running the browser process are reported through
/// [onExit].
abstract class Browser {
  String get name;

  /// The Observatory URL for this browser.
  ///
  /// Returns `null` for browsers that aren't running the Dart VM, or
  /// if the Observatory URL can't be found.
  Future<Uri>? get observatoryUrl => null;

  /// The remote debugger URL for this browser.
  ///
  /// Returns `null` for browsers that don't support remote debugging,
  /// or if the remote debugging URL can't be found.
  Future<Uri>? get remoteDebuggerUrl => null;

  /// A future that completes when the browser exits.
  ///
  /// If there's a problem starting or running the browser, this will complete
  /// with an error.
  Future<void> get onExit;

  /// Closes the browser
  ///
  /// Returns the same [Future] as [onExit], except that it won't emit
  /// exceptions.
  Future<void> close();
}

/// Interface for capturing screenshots from a browser.
abstract class ScreenshotManager {
  /// Capture a screenshot.
  ///
  /// Please read the details for the implementing classes.
  Future<Image> capture(math.Rectangle<num> region);

  /// Suffix to be added to the end of the filename.
  ///
  /// Example file names:
  /// - Chrome, no-suffix: backdrop_filter_clip_moved.actual.png
  /// - iOS Safari: backdrop_filter_clip_moved.iOS_Safari.actual.png
  String get filenameSuffix;
}
