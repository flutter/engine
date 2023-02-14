// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:path/path.dart' as pathlib;
import 'package:pool/pool.dart';

import '../environment.dart';
import '../exceptions.dart';
import '../pipeline.dart';
import '../utils.dart';

/// Compiles web tests and their dependencies into web_ui/build/.
///
/// Outputs of this step:
///
///  * canvaskit/   - CanvasKit artifacts
///  * assets/      - test fonts
///  * host/        - compiled test host page and static artifacts
///  * test/        - compiled test code
///  * test_images/ - test images copied from Skis sources.
class CompileTestsStep implements PipelineStep {
  CompileTestsStep({
    this.testFiles,
    this.isWasm = false
  });

  final List<FilePath>? testFiles;
  final bool isWasm;

  @override
  String get description => 'compile_tests';

  @override
  bool get isSafeToInterrupt => true;

  @override
  Future<void> interrupt() async {
    await cleanup();
  }

  @override
  Future<void> run() async {
    await compileTests(testFiles ?? findAllTests(), isWasm);
  }
}

/// Compiles the specified unit tests.
Future<void> compileTests(List<FilePath> testFiles, bool isWasm) async {
  final Stopwatch stopwatch = Stopwatch()..start();

  final TestsByRenderer sortedTests = sortTestsByRenderer(testFiles, isWasm);

  await Future.wait(<Future<void>>[
    if (sortedTests.htmlTests.isNotEmpty)
      _compileTestsInParallel(targets: sortedTests.htmlTests, isWasm: isWasm),
    if (sortedTests.canvasKitTests.isNotEmpty)
      _compileTestsInParallel(targets: sortedTests.canvasKitTests, renderer: Renderer.canvasKit, isWasm: isWasm),
    if (sortedTests.skwasmTests.isNotEmpty)
      _compileTestsInParallel(targets: sortedTests.skwasmTests, renderer: Renderer.skwasm, isWasm: isWasm),
  ]);

  stopwatch.stop();

  final int targetCount = sortedTests.numTargetsToCompile;
  print(
    'Built $targetCount tests in ${stopwatch.elapsedMilliseconds ~/ 1000} '
    'seconds using $_dart2jsConcurrency concurrent compile processes.',
  );
}

// Maximum number of concurrent dart2js processes to use.
int _dart2jsConcurrency = int.parse(io.Platform.environment['FELT_DART2JS_CONCURRENCY'] ?? '8');

final Pool _dart2jsPool = Pool(_dart2jsConcurrency);

/// Spawns multiple dart2js processes to compile [targets] in parallel.
Future<void> _compileTestsInParallel({
  required List<FilePath> targets,
  Renderer renderer = Renderer.html,
  bool isWasm = false,
}) async {
  final Stream<bool> results = _dart2jsPool.forEach(
    targets,
    (FilePath file) => compileUnitTest(file, renderer: renderer, isWasm: isWasm),
  );
  await for (final bool isSuccess in results) {
    if (!isSuccess) {
      throw ToolExit('Failed to compile tests.');
    }
  }
}

Future<bool> compileUnitTest(FilePath input, {required Renderer renderer, required bool isWasm}) async {
  return isWasm ? compileUnitTestToWasm(input, renderer: renderer)
    : compileUnitTestToJS(input, renderer: renderer);
}

/// Compiles one unit test using `dart2js`.
///
/// When building for CanvasKit we have to use extra argument
/// `DFLUTTER_WEB_USE_SKIA=true`.
///
/// Dart2js creates the following outputs:
/// - target.browser_test.dart.js
/// - target.browser_test.dart.js.deps
/// - target.browser_test.dart.js.map
/// under the same directory with test file. If all these files are not in
/// the same directory, Chrome dev tools cannot load the source code during
/// debug.
///
/// All the files under test already copied from /test directory to /build
/// directory before test are build. See [_copyFilesFromTestToBuild].
///
/// Later the extra files will be deleted in [_cleanupExtraFilesUnderTestDir].
Future<bool> compileUnitTestToJS(FilePath input, {required Renderer renderer}) async {
  // Compile to different directories for different renderers. This allows us
  // to run the same test in multiple renderers.
  final String targetFileName = pathlib.join(
    environment.webUiBuildDir.path,
    getBuildDirForRenderer(renderer),
    '${input.relativeToWebUi}.browser_test.dart.js',
  );

  final io.Directory directoryToTarget = io.Directory(pathlib.join(
      environment.webUiBuildDir.path,
      getBuildDirForRenderer(renderer),
      pathlib.dirname(input.relativeToWebUi)));

  if (!directoryToTarget.existsSync()) {
    directoryToTarget.createSync(recursive: true);
  }

  final List<String> arguments = <String>[
    'compile',
    'js',
    '--no-minify',
    '--disable-inlining',
    '--enable-asserts',

    // We do not want to auto-select a renderer in tests. As of today, tests
    // are designed to run in one specific mode. So instead, we specify the
    // renderer explicitly.
    '-DFLUTTER_WEB_AUTO_DETECT=false',
    '-DFLUTTER_WEB_USE_SKIA=${renderer == Renderer.canvasKit}',
    '-DFLUTTER_WEB_USE_SKWASM=${renderer == Renderer.skwasm}',

    '-O2',
    '-o',
    targetFileName, // target path.
    input.relativeToWebUi, // current path.
  ];

  final int exitCode = await runProcess(
    environment.dartExecutable,
    arguments,
    workingDirectory: environment.webUiRootDir.path,
  );

  if (exitCode != 0) {
    io.stderr.writeln('ERROR: Failed to compile test $input. '
        'Dart2js exited with exit code $exitCode');
    return false;
  } else {
    return true;
  }
}

Future<bool> compileUnitTestToWasm(FilePath input, {required Renderer renderer}) async {
  final String targetFileName = pathlib.join(
    environment.webUiBuildDir.path,
    getBuildDirForRenderer(renderer),
    '${input.relativeToWebUi}.browser_test.dart.wasm',
  );

  final io.Directory directoryToTarget = io.Directory(pathlib.join(
      environment.webUiBuildDir.path,
      getBuildDirForRenderer(renderer),
      pathlib.dirname(input.relativeToWebUi)));

  if (!directoryToTarget.existsSync()) {
    directoryToTarget.createSync(recursive: true);
  }

  final List<String> arguments = <String>[
    environment.dart2wasmSnapshotPath,

    '--dart-sdk=${environment.dartSdkDir.path}',
    '--enable-asserts',

    // We do not want to auto-select a renderer in tests. As of today, tests
    // are designed to run in one specific mode. So instead, we specify the
    // renderer explicitly.
    '-DFLUTTER_WEB_AUTO_DETECT=false',
    '-DFLUTTER_WEB_USE_SKIA=${renderer == Renderer.canvasKit}',
    '-DFLUTTER_WEB_USE_SKWASM=${renderer == Renderer.skwasm}',

    if (renderer == Renderer.skwasm)
      ...<String>[
        '--import-shared-memory',
        '--shared-memory-max-pages=32768',
      ],

    input.relativeToWebUi, // current path.
    targetFileName, // target path.
  ];

  final int exitCode = await runProcess(
    environment.dartAotRuntimePath,
    arguments,
    workingDirectory: environment.webUiRootDir.path,
  );

  if (exitCode != 0) {
    io.stderr.writeln('ERROR: Failed to compile test $input. '
        'dart2wasm exited with exit code $exitCode');
    return false;
  } else {
    return true;
  }
}
