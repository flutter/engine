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
import 'package:kernel/target/flutter_fasta.dart';
import 'package:kernel/target/targets.dart';
import 'package:usage/lib/uuid/uuid.dart';

ArgParser argParser = new ArgParser(allowTrailingOptions: true)
  ..addFlag('train',
      help: 'Run through sample command line to produce snapshot',
      negatable: false)
  ..addFlag('incremental',
      help: 'Run compiler in incremental mode', defaultsTo: false)
  ..addOption('sdk-root',
      help: 'Path to sdk root',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk')
  ..addOption('platform-kernel-dill',
      help: 'Path to platform kernel dill file',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk/platform.dill');

String usage = '''
Usage: server [options] [input.dart]

If input dart source code is provided on the command line, then the server 
compiles it, generates dill file and exits.
If no input dart source is provided on the command line, server waits for 
instructions from stdin.

Instructions:
- compile <input.dart>
- recompile <boundary-key>
<invalidate-file1.dart>
<invalidate-file2.dart>
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

main(List<String> args) async {
  ArgResults options = argParser.parse(args);
  if (options['train']) {
    exit(0);
  }

  if (options.rest.isNotEmpty) {
    await compile(options.rest[0], options);
    exit(0);
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
          await compile(filename, options);
        } else if (string.startsWith('recompile ')) {
          boundaryKey = string.substring('recompile '.length);
          state = State.RECOMPILE_LIST;
        } else if (string == 'accept') {
          ikg.acceptLastDelta();
        } else if (string == 'reject') {
          ikg.rejectLastDelta();
        } else if (string == 'quit') {
          exit(0);
        }
        break;
      case State.RECOMPILE_LIST:
        if (string == boundaryKey) {
          recompileDelta();
          state = State.READY_FOR_INSTRUCTION;
        } else {
          ikg.invalidate(Uri.parse(string));
        }
        break;
    }
  });
}

IncrementalKernelGenerator ikg;
String filename;

compile(String filename, ArgResults options) async {
  var program;
  String boundaryKey = new Uuid().generateV4();
  print("result $boundaryKey");
  if (options['incremental']) {
    final CompilerOptions compilerOptions = new CompilerOptions()
      ..sdkRoot = Uri.base.resolve(options['sdk-root'])
      ..strongMode = false
      ..target = new FlutterTarget(new TargetFlags())
      ..onError = (CompilationMessage message) {
        print(message);
      };
    ikg = await IncrementalKernelGenerator.newInstance(
        compilerOptions, Uri.base.resolve(filename));
    var deltaProgram = await ikg.computeDelta();
    program = deltaProgram.newProgram;
  } else {
    final CompilerOptions compilerOptions = new CompilerOptions()
      ..sdkRoot = Uri.base.resolve(options['sdk-root'])
      ..chaseDependencies = true
      ..compileSdk = true
      ..linkedDependencies = <Uri>[
        Uri.base.resolve(options['platform-kernel-dill'])
      ]
      ..target = new FlutterFastaTarget(new TargetFlags())
      ..onError = (CompilationMessage message) {
        print(message);
      };
    program = await kernelForProgram(new Uri.file(filename), compilerOptions);
  }
  if (program != null) {
    final String kernelBinaryFilename = filename + ".dill";
    await writeProgramToBinary(program, kernelBinaryFilename);
    print(boundaryKey + " " + kernelBinaryFilename);
  } else {
    print(boundaryKey);
  }
}

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
