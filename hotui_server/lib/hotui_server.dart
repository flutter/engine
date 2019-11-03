// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:kernel/binary/limited_ast_to_binary.dart';
import 'package:path/path.dart' as path;
import 'package:args/args.dart';
import 'package:kernel/ast.dart';
import 'package:kernel/binary/ast_to_binary.dart';

import 'package:vm/incremental_compiler.dart';
import 'package:front_end/src/api_unstable/vm.dart'; // ignore: implementation_imports
import 'package:vm/kernel_front_end.dart'
    show
        convertFileOrUriArgumentToUri,
        createFrontEndFileSystem,
        createFrontEndTarget,
        setVMEnvironmentDefines;

// These options are a subset of the overall frontend server options.
final ArgParser argParser = ArgParser()
  ..addFlag('train')
  ..addOption('sdk-root',
      help: 'Path to sdk root',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk')
  ..addOption('platform', help: 'Platform kernel filename')
  ..addOption('packages',
      help: '.packages file to use for compilation', defaultsTo: null)
  ..addOption('target-model',
      help: 'Target model that determines what core libraries are available',
      allowed: <String>[
        'vm',
        'flutter',
        'flutter_runner',
        'dart_runner',
        // 'dartdevc' does not support hot UI.
      ],
      defaultsTo: 'vm')
  ..addMultiOption('filesystem-root',
      help: 'File path that is used as a root in virtual filesystem used in'
          ' compiled kernel files. When used --output-dill should be provided'
          ' as well.',
      hide: true)
  ..addOption('filesystem-scheme',
      help:
          'Scheme that is used in virtual filesystem set up via --filesystem-root'
          ' option',
      defaultsTo: 'org-dartlang-root',
      hide: true)
  ..addFlag('verbose', help: 'Enables verbose output from the compiler.')
  ..addOption('initialize-from-dill',
      help: 'Normally the output dill is used to specify which dill to '
          'initialize from, but it can be overwritten here.',
      defaultsTo: null,
      hide: true)
  ..addOption('target',
      help:
          'The file URI of the library containing the applications main method')
  ..addOption('output-dill',
      help: 'Output path for the generated dill', defaultsTo: null);

const String kReadyMessage = 'READY';
const String kFailedMessage = 'FAILED';

// This implementation has three known limitations
//
// * The line and column numbers of the patched method are not currently
//   correct. This might need an update to the evaulate expression logic.
// * The full component is only compiled once during startup. Since we only
//   send a component containing a single changed library, this will usually
//   work. Long term, this needs to be aware of the normal invalidation
//  cycle.
// * This writes out an incremental dill file to the provided path.
//   As a performance improvement, it should be written directly into an
//   http request body for the vmservice to load from.
Future<void> main(List<String> args) async {
  final ArgResults options = argParser.parse(args);

  // Since this is a short-lived package for experimentation it is not
  // worth training.
  if (options['train']) {
    return;
  }
  final Uri entrypointUri = Uri.parse(options['target']);

  // Setup compiler configuration from arguments.
  final FileSystem fileSystem = createFrontEndFileSystem(
    options['filesystem-scheme'],
    options['filesystem-root'],
  );
  final Uri sdkRoot = Uri.base.resolve(options['sdk-root']);
  final String platformKernelDill =
      options['platform'] ?? 'platform_strong.dill';
  final CompilerOptions compilerOptions = CompilerOptions()
    ..sdkRoot = sdkRoot
    ..fileSystem = fileSystem
    ..packagesFileUri =
        convertFileOrUriArgumentToUri(fileSystem, options['packages'])
    ..sdkSummary = sdkRoot.resolve(platformKernelDill)
    ..verbose = options['verbose']
    ..target = createFrontEndTarget(
      options['target-model'],
      trackWidgetCreation: true,
    );
  setVMEnvironmentDefines(<String, String>{}, compilerOptions);

  // Create incremental compiler.
  final Uri initializeFromDillUri =
      Uri.file(options['initialize-from-dill'] ?? options['output-dill']);
  final IncrementalCompiler incrementalCompiler = IncrementalCompiler(
    compilerOptions,
    entrypointUri,
    initializeFromDillUri: initializeFromDillUri,
  );

  // Compile the initial component
  final Component component =
      await incrementalCompiler.compile(entryPoint: entrypointUri);
  component.computeCanonicalNames();
  incrementalCompiler.accept();

  // Ready for hot UI.
  HotUIService(
    incrementalCompiler,
    component,
    File(initializeFromDillUri.toFilePath()).parent,
  );
}

/// This service avoids re-parsing dart code by maintaining a single full
/// component and performing updates to this component in place.
///
/// Hot ui works by specifiny a libraryId, classId, methodId, and new method
/// body. The hot UI service uses this information to produce a partial kernel
/// file containing the updated code. Notably, this is done without modifying
/// source on disk using an inplace edit of the kernel AST.
///
/// Since the hot ui service may accept any libraryId, it must hold a full
/// component in memory. Then, after modifying the AST, it manually trims the
/// component to contain only the modified library.
class HotUIService {
  HotUIService(this.incrementalCompiler, this.component, this.outputDirectory) {
    stdin
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen(onMessage);
    print(kReadyMessage);
  }

  final IncrementalCompiler incrementalCompiler;
  final Component component;
  final Directory outputDirectory;

  // Example invocation:
  //
  //    {"class":"_DemoItem","method":"build","library":"package:flutter_gallery/gallery/home.dart","methodBody":"Text('Hello, World')"}
  Future<void> reloadMethod(String libraryId, String classId, String methodId,
      String methodBody) async {
    final Procedure procedure = await incrementalCompiler.compileExpression(
        methodBody,
        // This is not yet flexible for non-build expressions.
        <String>['context'],
        <String>['BuildContext'],
        libraryId,
        classId,
        false);
    component.transformChildren(
        BodyReplacementTransformer(libraryId, classId, methodId, procedure));
    final Library modifiedLibrary = component.libraries.firstWhere(
        (Library library) => library.importUri.toString() == libraryId,
        orElse: () => null);
    if (modifiedLibrary == null) {
      throw Exception('Could not find library with id: $libraryId');
    }
    final Component partialComponent = Component(libraries: <Library>[
      modifiedLibrary,
    ]);
    partialComponent.unbindCanonicalNames();
    partialComponent.computeCanonicalNames();
    final IOSink sink =
        File(path.join(outputDirectory.path, 'hotui.dill')).openWrite();
    final BinaryPrinter printer =
        LimitedBinaryPrinter(sink, (_) => true, false);
    printer.writeComponentFile(partialComponent);
    await sink.close();
  }

  // Expects newline denominated JSON object.
  void onMessage(String line) {
    Map<String, Object> message;
    try {
      message = json.decode(line);
    } on FormatException catch (error) {
      stderr.writeln(error.toString());
      print(kFailedMessage);
    }
    final String libraryId = message['library'];
    final String classId = message['class'];
    final String methodId = message['method'];
    final String methodBody = message['methodBody'];

    reloadMethod(libraryId, classId, methodId, methodBody).then((void result) {
      print(kReadyMessage);
    }).catchError((dynamic error, StackTrace stackTrace) {
      stderr.writeln(error.toString());
      stderr.writeln(stackTrace.toString());
      print(kFailedMessage);
    });
  }
}

/// Replaces the body of method [methodId] on class [classId] in library
/// [libraryId] with [procedure].
class BodyReplacementTransformer extends Transformer {
  BodyReplacementTransformer(
    this.libraryId,
    this.classId,
    this.methodId,
    this.procedure,
  );

  final String libraryId;
  final String classId;
  final String methodId;
  final Procedure procedure;

  @override
  Library visitLibrary(Library node) {
    if (node.importUri.toString() == libraryId) {
      return super.visitLibrary(node);
    }
    return node;
  }

  @override
  Class visitClass(Class clazz) {
    if (clazz.name == classId) {
      return super.visitClass(clazz);
    }
    return clazz;
  }

  @override
  TreeNode visitProcedure(Procedure node) {
    if (node.name.name == methodId && node.kind == ProcedureKind.Method) {
      return procedure;
    }
    return super.visitProcedure(node);
  }
}

