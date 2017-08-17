library frontend_server;

import 'dart:convert';
import 'dart:io';

import 'package:args/args.dart';
import 'package:front_end/compilation_message.dart';

import 'package:front_end/incremental_kernel_generator.dart';
import 'package:front_end/compiler_options.dart';
import 'package:front_end/kernel_generator.dart';
import 'package:kernel/ast.dart';
import 'package:kernel/kernel.dart';
import 'package:kernel/target/flutter.dart';
import 'package:kernel/target/targets.dart';
import 'package:usage/uuid/uuid.dart';

ArgParser argParser = new ArgParser(allowTrailingOptions: true)
  ..addFlag('train',
      help: 'Run through sample command line to produce snapshot',
      negatable: false)
  ..addFlag('incremental',
      help: 'Run compiler in incremental mode', defaultsTo: false)
  ..addOption('sdk-root',
      help: 'Path to sdk root',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk');

String usage = '''
Usage: server [options] [input.dart]

If input dart source code is provided on the command line, then the server 
compiles it, generates dill file and exits.
If no input dart source is provided on the command line, server waits for 
instructions from stdin.

Instructions:
- compile <input.dart>
- recompile <boundary-key>
<path/to/updated/file1.dart>
<path/to/updated/file2.dart>
...
<boundary-key>
- accept
- reject
- quit

Output:
- result <boundary-key>
<compiler output>
<boundary-key> [<output.dill>]

Options:
${argParser.usage}
''';

enum State { READY_FOR_INSTRUCTION, RECOMPILE_LIST }

abstract class CompilerInterface {
  compile(String filename, ArgResults options);
  recompileDelta();
  acceptLastDelta();
  rejectLastDelta();
  invalidate(Uri uri);
}

class FrontendCompiler implements CompilerInterface {
  IncrementalKernelGenerator ikg;
  String filename;

  @override
  compile(String filename, ArgResults options) async {
    var program;
    String boundaryKey = new Uuid().generateV4();
    print("result $boundaryKey");
    Uri sdkRoot = _ensureFolderPath(options['sdk-root']);
    final CompilerOptions compilerOptions = new CompilerOptions()
      ..sdkRoot = sdkRoot
      ..strongMode = false
      ..target = new FlutterTarget(new TargetFlags())
      ..onError = (CompilationMessage message) {
        print(message);
      };
    if (options['incremental']) {
      ikg = await IncrementalKernelGenerator.newInstance(
          compilerOptions, Uri.base.resolve(filename));
      var deltaProgram = await ikg.computeDelta();
      program = deltaProgram.newProgram;
    } else {
      // TODO(aam): Remove linkedDependencies once platform is directly embedded
      // into VM snapshot and http://dartbug.com/30111 is fixed.
      compilerOptions.linkedDependencies = <Uri>[
        sdkRoot.resolve('platform.dill')
      ];
      program = await kernelForProgram(Uri.base.resolve(filename), compilerOptions);
    }
    if (program != null) {
      final String kernelBinaryFilename = filename + ".dill";
      await writeProgramToBinary(program, kernelBinaryFilename);
      print(boundaryKey + " " + kernelBinaryFilename);
    } else {
      print(boundaryKey);
    }
  }

  @override
  recompileDelta() async {
    ikg.computeDelta();
    String boundaryKey = new Uuid().generateV4();
    print("result $boundaryKey");
    DeltaProgram deltaProgram = await ikg.computeDelta();
    Program program = deltaProgram.newProgram;
    final String kernelBinaryFilename = filename + ".dill";
    await writeProgramToBinary(program, kernelBinaryFilename);
    print(boundaryKey);
    return kernelBinaryFilename;
  }

  @override
  acceptLastDelta() {
    ikg.acceptLastDelta();
  }

  @override
  rejectLastDelta() {
    ikg.rejectLastDelta();
  }

  @override
  invalidate(Uri uri) {
    ikg.invalidate(uri);
  }

  Uri _ensureFolderPath(String path) {
    if (!path.endsWith('/')) path = '$path/';
    return Uri.base.resolve(path);
  }
}

// compiler is an optional parameter so it can be replaced with mocked version
// for testing.
starter(List<String> args, {CompilerInterface compiler}) async {
  ArgResults options = argParser.parse(args);
  if (options['train']) {
    return 0;
  }

  compiler ??= new FrontendCompiler();

  if (options.rest.isNotEmpty) {
    await compiler.compile(options.rest[0], options);
    return 0;
  }

  State state = State.READY_FOR_INSTRUCTION;
  String boundaryKey;
  print('Frontend server is ready.');
  stdin
      .transform(UTF8.decoder)
      .transform(new LineSplitter())
      .listen((String string) async {
    switch (state) {
      case State.READY_FOR_INSTRUCTION:
        if (string.startsWith('compile ')) {
          String filename = string.substring('compile '.length);
          await compiler.compile(filename, options);
        } else if (string.startsWith('recompile ')) {
          boundaryKey = string.substring('recompile '.length);
          state = State.RECOMPILE_LIST;
        } else if (string == 'accept') {
          compiler.acceptLastDelta();
        } else if (string == 'reject') {
          compiler.rejectLastDelta();
        } else if (string == 'quit') {
          exit(0);
        }
        break;
      case State.RECOMPILE_LIST:
        if (string == boundaryKey) {
          compiler.recompileDelta();
          state = State.READY_FOR_INSTRUCTION;
        } else {
          compiler.invalidate(Uri.base.resolve(string));
        }
        break;
    }
  });
}
