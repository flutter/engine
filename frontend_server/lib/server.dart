// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library frontend_server;

import 'dart:async';
import 'dart:io' hide FileSystemEntity;

import 'package:args/args.dart';
import 'package:path/path.dart' as path;
import 'package:vm/frontend_server.dart' as frontend show FrontendCompiler, argParser, usage;

import 'package:flutter_kernel_transformers/track_widget_constructor_locations.dart';

/// Wrapper around [FrontendCompiler] that adds [wdgetCreatorTracker] kernel
/// transformation to the compilation.
class _FlutterFrontendCompiler {
  final frontend.CompilerInterface _compiler;

  _FlutterFrontendCompiler(StringSink output, {bool trackWidgetCreation: false}):
      _compiler = new frontend.FrontendCompiler(output,
          transformer: (trackWidgetCreation ? new WidgetCreatorTracker() : null));

  @override
  Future<Null> compile(String filename, ArgResults options) async {
    return _compiler.compile(filename, options);
  }

  @override
  Future<Null> recompileDelta({String filename}) async {
    return _compiler.recompileDelta(filename: filename);
  }

  @override
  void acceptLastDelta() {
    _compiler.acceptLastDelta();
  }

  @override
  void invalidate(Uri uri) {
    _compiler.invalidate(uri);
  }

  @override
  void resetIncrementalCompiler() {
    _compiler.resetIncrementalCompiler();
  }
}

/// Entry point for this module, that creates `_FrontendCompiler` instance and
/// processes user input.
/// `compiler` is an optional parameter so it can be replaced with mocked
/// version for testing.
Future<int> starter(
    List<String> args, {
      _FlutterFrontendCompiler compiler,
      Stream<List<int>> input,
      StringSink output,
    }) async {
  ArgResults options;
  frontend.argParser
    ..addFlag('track-widget-creation',
      help: 'Run a kernel transformer to track creation locations for widgets.',
      defaultsTo: false);

  try {
    options = frontend.argParser.parse(args);
  } catch (error) {
    print('ERROR: $error\n');
    print(frontend.usage);
    return 1;
  }

  if (options['train']) {
    final String sdkRoot = options['sdk-root'];
    final Directory temp = Directory.systemTemp.createTempSync('train_frontend_server');
    try {
      final String outputTrainingDill = path.join(temp.path, 'app.dill');
      options = frontend.argParser.parse(<String>[
        '--incremental',
        '--sdk-root=$sdkRoot',
        '--output-dill=$outputTrainingDill',
        '--target=flutter']);
      compiler ??= new _FlutterFrontendCompiler(output, trackWidgetCreation: true);

      await compiler.compile(Platform.script.toFilePath(), options);
      compiler.acceptLastDelta();
      await compiler.recompileDelta();
      compiler.acceptLastDelta();
      compiler.resetIncrementalCompiler();
      await compiler.recompileDelta();
      compiler.acceptLastDelta();
      await compiler.recompileDelta();
      compiler.acceptLastDelta();
      return 0;
    } finally {
      temp.deleteSync(recursive: true);
    }
  }

  compiler ??= new _FlutterFrontendCompiler(output, trackWidgetCreation: options['track-widget-creation']);

  if (options.rest.isNotEmpty) {
    await compiler.compile(options.rest[0], options);
    return 0;
  }

  frontend.listenAndCompile(input ?? stdin, options, () { exit(0); } );
  return 0;
}
