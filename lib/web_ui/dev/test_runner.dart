// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:path/path.dart' as path;

import 'package:watcher/src/watch_event.dart';

import 'environment.dart';
import 'felt_config.dart';
import 'pipeline.dart';
import 'steps/compile_bundle_step.dart';
import 'steps/copy_artifacts_step.dart';
import 'steps/run_suite_step.dart';
import 'suite_filter.dart';
import 'utils.dart';

/// Runs tests.
class TestCommand extends Command<bool> with ArgUtils<bool> {
  TestCommand() {
    argParser
      ..addFlag(
        'debug',
        help: 'Pauses the browser before running a test, giving you an '
            'opportunity to add breakpoints or inspect loaded code before '
            'running the code.',
      )
      ..addFlag(
        'verbose',
        abbr: 'v',
        help: 'Enable verbose output.'
      )
      ..addFlag(
        'watch',
        abbr: 'w',
        help: 'Run in watch mode so the tests re-run whenever a change is '
            'made.',
      )
      ..addFlag(
        'list',
        help:
            'Lists the bundles that would be compiled and the suites that '
            'will be run as part of this invocation, without actually '
            'compiling or running them.'
      )
      ..addFlag(
        'compile',
        help:
            'Compile test bundles. If this is specified on its own, we will '
            'only compile and not run the suites.'
      )
      ..addFlag(
        'run',
        help:
            'Run test suites. If this is specified on its own, we will only '
            'run the suites and not compile the bundles.'
      )
      ..addFlag(
        'require-skia-gold',
        help:
            'Whether we require Skia Gold to be available or not. When this '
            'flag is true, the tests will fail if Skia Gold is not available.',
      )
      ..addFlag(
        'update-screenshot-goldens',
        help:
            'When running screenshot tests writes them to the file system into '
            '.dart_tool/goldens. Use this option to bulk-update all screenshots, '
            'for example, when a new browser version affects pixels.',
      )
      ..addOption(
        'browser',
        defaultsTo: 'chrome',
        help: 'An option to choose a browser to run the tests. By default '
              'tests run in Chrome.',
      )
      ..addFlag(
        'fail-early',
        help: 'If set, causes the test runner to exit upon the first test '
              'failure. If not set, the test runner will continue running '
              'test despite failures and will report them after all tests '
              'finish.',
      )
      ..addOption(
        'canvaskit-path',
        help: 'Optional. The path to a local build of CanvasKit to use in '
              'tests. If omitted, the test runner uses the default CanvasKit '
              'build.',
      )
      ..addFlag(
        'wasm',
        help: 'Whether the test we are running are compiled to webassembly.'
      )
      ..addFlag(
        'use-local-canvaskit',
        help: 'Optional. Whether or not to use the locally built version of '
              'CanvasKit in the tests.',
      );
  }

  @override
  final String name = 'test';

  @override
  final String description = 'Run tests.';

  bool get isWatchMode => boolArg('watch');

  bool get isList => boolArg('list');

  bool get failEarly => boolArg('fail-early');

  bool get isWasm => boolArg('wasm');

  /// Whether to start the browser in debug mode.
  ///
  /// In this mode the browser pauses before running the test to allow
  /// you set breakpoints or inspect the code.
  bool get isDebug => boolArg('debug');

  bool get isVerbose => boolArg('verbose');

  /// Paths to targets to run, e.g. a single test.
  List<String> get targets => argResults!.rest;

  /// The target test files to run.
  List<FilePath> get targetFiles => targets.map((String t) => FilePath.fromCwd(t)).toList();

  /// Whether all tests should run.
  bool get runAllTests => targets.isEmpty;

  /// The name of the browser to run tests in.
  String get browserName => stringArg('browser');

  /// When running screenshot tests, require Skia Gold to be available and
  /// reachable.
  bool get requireSkiaGold => boolArg('require-skia-gold');

  /// When running screenshot tests writes them to the file system into
  /// ".dart_tool/goldens".
  bool get doUpdateScreenshotGoldens => boolArg('update-screenshot-goldens');

  /// Path to a CanvasKit build. Overrides the default CanvasKit.
  String? get overridePathToCanvasKit => argResults!['canvaskit-path'] as String?;

  /// Whether or not to use the locally built version of CanvasKit.
  bool get useLocalCanvasKit => boolArg('use-local-canvaskit');

  List<SuiteFilter> get suiteFilters {
    return <SuiteFilter>[
      PlatformSuiteFilter()
      // TODO(jacksongardner): Add more filters
      // Add browser filter from CLI
      // Add suite filter from CLI
      // Add compiler filter from CLI
      // Add file filter from CLI
      // Add renderer filter from CLI
    ];
  }

  List<TestSuite> _filterTestSuites(List<TestSuite> suites) {
    if (isVerbose) {
      print('Filtering suites...');
    }
    final List<SuiteFilter> filters = suiteFilters;
    final List<TestSuite> filteredSuites = suites.where((TestSuite suite) {
      for (final SuiteFilter filter in filters) {
        final SuiteFilterResult result = filter.filterSuite(suite);
        if (!result.isAccepted) {
          if (isVerbose) {
            print('  ${suite.name.ansiCyan} rejected for reason: ${result.rejectReason}');
          }
          return false;
        }
      }
      return true;
    }).toList();
    return filteredSuites;
  }

  List<TestBundle> _filterBundlesForSuites(List<TestBundle> bundles, List<TestSuite> suites) {
    final Set<TestBundle> seenBundles = 
      Set<TestBundle>.from(suites.map((TestSuite suite) => suite.testBundle));
    return bundles.where((TestBundle bundle) => seenBundles.contains(bundle)).toList();
  }

  ArtifactDependencies _artifactsForSuites(List<TestSuite> suites) {
    return suites.fold(ArtifactDependencies.none(), 
      (ArtifactDependencies deps, TestSuite suite) => deps | suite.artifactDependencies);
  }

  @override
  Future<bool> run() async {
    final FeltConfig config = FeltConfig.fromFile(
      path.join(environment.webUiTestDir.path, 'felt_config.yaml')
    );
    final List<TestSuite> filteredSuites = _filterTestSuites(config.testSuites);
    final List<TestBundle> bundles = _filterBundlesForSuites(config.testBundles, filteredSuites);
    final ArtifactDependencies artifacts = _artifactsForSuites(config.testSuites);
    if (isList || isVerbose) {
      print('Suites:');
      for (final TestSuite suite in filteredSuites) {
        print('  ${suite.name.ansiCyan}');
      }
      print('Bundles:');
      for (final TestBundle bundle in bundles) {
        print('  ${bundle.name.ansiMagenta}');
      }
      print('Artifacts:');
      if (artifacts.canvasKit) {
        print('  canvaskit'.ansiYellow);
      }
      if (artifacts.canvasKitChromium) {
        print('  canvaskit_chromium'.ansiYellow);
      }
      if (artifacts.skwasm) {
        print('  skwasm'.ansiYellow);
      }
    }
    if (isList) {
      return true;
    }

    bool shouldRun = boolArg('run');
    bool shouldCompile = boolArg('compile');
    if (!shouldRun && !shouldCompile) {
      // If neither is specified, we should assume we need to both compile and run.
      shouldRun = true;
      shouldCompile = true;
    }
    final Pipeline testPipeline = Pipeline(steps: <PipelineStep>[
      if (isWatchMode) ClearTerminalScreenStep(),
      CopyArtifactsStep(artifacts),
      if (shouldCompile)
        for (final TestBundle bundle in bundles)
          CompileBundleStep(bundle: bundle),
      if (shouldRun)
        for (final TestSuite suite in filteredSuites)
          RunSuiteStep(
            suite, 
            isDebug: isDebug,
            doUpdateScreenshotGoldens: doUpdateScreenshotGoldens,
            requireSkiaGold: requireSkiaGold,
            overridePathToCanvasKit: overridePathToCanvasKit,
          ),
    ]);

    try {
      await testPipeline.run();
      if (isWatchMode) {
        print('');
        print('Initial test succeeded!');
      }
    } catch(error, stackTrace) {
      if (isWatchMode) {
        // The error is printed but not rethrown in watch mode because
        // failures are expected. The idea is that the developer corrects the
        // error, saves the file, and the pipeline reruns.
        print('');
        print('Initial test failed!\n');
        print(error);
        print(stackTrace);
      } else {
        rethrow;
      }
    }

    if (isWatchMode) {
      final FilePath dir = FilePath.fromWebUi('');
      print('');
      print(
          'Watching ${dir.relativeToCwd}/lib and ${dir.relativeToCwd}/test to re-run tests');
      print('');
      await PipelineWatcher(
          dir: dir.absolute,
          pipeline: testPipeline,
          ignore: (WatchEvent event) {
            // Ignore font files that are copied whenever tests run.
            if (event.path.endsWith('.ttf')) {
              return true;
            }

            // React to changes in lib/ and test/ folders.
            final String relativePath =
                path.relative(event.path, from: dir.absolute);
            if (path.isWithin('lib', relativePath) ||
                path.isWithin('test', relativePath)) {
              return false;
            }

            // Ignore anything else.
            return true;
          }).start();
    }
    return true;
  }
}

/// Clears the terminal screen and places the cursor at the top left corner.
///
/// This works on Linux and Mac. On Windows, it's a no-op.
class ClearTerminalScreenStep implements PipelineStep {
  @override
  String get description => 'clearing terminal screen';

  @override
  bool get isSafeToInterrupt => false;

  @override
  Future<void> interrupt() async {}

  @override
  Future<void> run() async {
    if (!io.Platform.isWindows) {
      // See: https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences
      print('\x1B[2J\x1B[1;2H');
    }
  }
}
