// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:args/args.dart';
import 'package:kernel/ast.dart';
import 'package:kernel/binary/ast_to_binary.dart';
import 'package:vm_service/vm_service.dart' as vm_service;
import 'package:vm_service/vm_service_io.dart';
import 'package:vm/incremental_compiler.dart';
import 'package:front_end/src/api_unstable/vm.dart'; // ignore: implementation_imports
import 'package:vm/kernel_front_end.dart'
    show
        convertFileOrUriArgumentToUri,
        createFrontEndFileSystem;

// These options are a subset of the overall frontend server options.
final ArgParser argParser = ArgParser()
  ..addOption('sdk-root',
      help: 'Path to sdk root',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk')
  ..addOption('platform', help: 'Platform kernel filename')
  ..addOption('packages',
      help: '.packages file to use for compilation', defaultsTo: null)
  ..addOption('target',
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
      help: 'Output path for the generated dill', defaultsTo: null)
  // hot UI specific options
  ..addOption('vmservice', help: 'The URI of the vmservice to connect to')
  ..addOption('devfs-uri', help: 'The URI of the devFS main dill')
  ..addOption('devfs', help: 'The name of the devFS to use for hot reload')
  ..addOption('isolate-Id', help: 'The isolate id to be reloaded');

const String kReadyMessage = 'READY';
const String kFailedMessage = 'FAILED';

Future<void> main(List<String> args) async {
  final ArgResults options = argParser.parse(args);
  final String vmServiceUri = options['vmservice'];
  final Uri entrypointUri = Uri.parse(options['target']);
  final String devFs = options['devfs'];
  final String devfsUri = options['devfs-uri'];
  final String httpAddress =
      vmServiceUri.replaceFirst('ws', 'http').replaceFirst('/ws', '');
  final String isolateId = options['isolate-Id'];

  final vm_service.VmService vmService = await vmServiceConnectUri(vmServiceUri);
  final HttpClient httpClient = HttpClient();

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
    ..verbose = options['verbose'];

  // Create incremental compiler.
  final Uri initializeFromDillUri =
      Uri.file(options['initialize-from-dill'] ?? options['output-dill']);
  final IncrementalCompiler incrementalCompiler = IncrementalCompiler(
    compilerOptions,
    entrypointUri,
    initializeFromDillUri: initializeFromDillUri,
  );

  // Compile the initial component
  final Component component = await incrementalCompiler.compile();
  component.computeCanonicalNames();
  incrementalCompiler.accept();

  // Ready for hot UI.
  HotUIService(vmService, httpClient, incrementalCompiler, component, devFs,
      httpAddress, devfsUri, isolateId);
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
  HotUIService(
      this.vmService,
      this.httpClient,
      this.incrementalCompiler,
      this.component,
      this.devFs,
      this.httpAddress,
      this.devfsUri,
      this.isolateId) {

    stdin
        .transform(utf8.decoder)
        .transform(const LineSplitter())
        .listen(onMessage);
    print(kReadyMessage);
  }

  final vm_service.VmService vmService;
  final HttpClient httpClient;
  final IncrementalCompiler incrementalCompiler;
  final Component component;
  final String devFs;
  final String httpAddress;
  final String devfsUri;
  final String isolateId;

  Future<void> reloadMethod(String libraryId, String classId, String methodId,
      String methodBody) async {
    final Stopwatch sw = Stopwatch()..start();
    final Procedure procedure = await incrementalCompiler
      .compileExpression(methodBody, <String>['context'], <String>['BuildContext'], libraryId, classId, false);
    component.transformChildren(BodyReplacementTransformer(libraryId, classId, methodId, procedure));
    final Component partialComponent = Component(libraries: <Library>[
      component.libraries.firstWhere((Library library) => library.importUri.toString() == libraryId)
    ]);
    stderr.writeln('compiled expression in ${sw.elapsedMicroseconds}');

    // Use the HTTP request as the sink for serializing the partial dill.
    final HttpClientRequest request =
        await httpClient.putUrl(Uri.parse(httpAddress));
    request.headers.removeAll(HttpHeaders.acceptEncodingHeader);
    request.headers.add('dev_fs_name', devFs);
    request.headers.add(
        'dev_fs_uri_base64',
        base64
            .encode(utf8.encode('${devfsUri}lib/main.dart.incremental.dill')));
    final BinaryPrinter binaryPrinter = BinaryPrinter(request);
    binaryPrinter.writeComponentFile(partialComponent);
    await request.close();
    await vmService.reloadSources(isolateId);

    // Invoke reassemble.
    await vmService.callServiceExtension('flutter.ext.reassemble');
  }



  // Expects newline denominated JSON object.
  void onMessage(String line) {
    final Map<String, Object> message = json.decode(line);
    final String libraryId = message['library'];
    final String classId = message['classId'];
    final String methodId = message['methodId'];
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
