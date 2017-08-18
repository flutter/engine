library frontend_server;

import 'dart:async';
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

ArgParser _argParser = new ArgParser(allowTrailingOptions: true)
  ..addFlag('train',
      help: 'Run through sample command line to produce snapshot',
      negatable: false)
  ..addFlag('incremental',
      help: 'Run compiler in incremental mode', defaultsTo: false)
  ..addOption('sdk-root',
      help: 'Path to sdk root',
      defaultsTo: '../../out/android_debug/flutter_patched_sdk');

String _usage = '''
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
${_argParser.usage}
''';

enum _State {
  READY_FOR_INSTRUCTION,
  RECOMPILE_LIST
}

/// Actions that every compiler should implement.
abstract class CompilerInterface {
  /// Compile given Dart program identified by `filename` with given list of
  /// `options`.
  Future<Null> compile(String filename, ArgResults options);
  /// Assuming some Dart program was previously compiled, recompile it again
  /// taking into account some changed(invalidated) sources.
  Future<Null> recompileDelta();
  /// Accept results of previous compilation so that next recompilation cycle
  /// won't recompile sources that were previously reported as changed.
  void acceptLastDelta();
  /// Reject results of previous compilation. Next recompilation cycle will
  /// recompile sources indicated as changed.
  void rejectLastDelta();
  /// This let's compiler know that source file identifed by `uri` was changed.
  void invalidate(Uri uri);
}

class _FrontendCompiler implements CompilerInterface {
  IncrementalKernelGenerator _ikg;
  String _filename;

  @override
  Future<Null> compile(String filename, ArgResults options) async {
    final String boundaryKey = new Uuid().generateV4();
    print("result $boundaryKey");
    final Uri sdkRoot = _ensureFolderPath(options['sdk-root']);
    final CompilerOptions compilerOptions = new CompilerOptions()
      ..sdkRoot = sdkRoot
      ..strongMode = false
      ..target = new FlutterTarget(new TargetFlags())
      ..onError = (CompilationMessage message) {
        print(message);
      };
    Program program;
    if (options['incremental']) {
      _ikg = await IncrementalKernelGenerator.newInstance(
          compilerOptions, Uri.base.resolve(filename));
      final DeltaProgram deltaProgram = await _ikg.computeDelta();
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
    return null;
  }

  @override
  Future<Null> recompileDelta() async {
    _ikg.computeDelta();
    final String boundaryKey = new Uuid().generateV4();
    print("result $boundaryKey");
    final DeltaProgram deltaProgram = await _ikg.computeDelta();
    final Program program = deltaProgram.newProgram;
    final String kernelBinaryFilename = _filename + ".dill";
    await writeProgramToBinary(program, kernelBinaryFilename);
    print(boundaryKey);
    return null;
  }

  @override
  void acceptLastDelta() {
    _ikg.acceptLastDelta();
  }

  @override
  void rejectLastDelta() {
    _ikg.rejectLastDelta();
  }

  @override
  void invalidate(Uri uri) {
    _ikg.invalidate(uri);
  }

  Uri _ensureFolderPath(String path) {
    if (!path.endsWith('/')) {
      path = '$path/';
    }
    return Uri.base.resolve(path);
  }
}

/// Entry point for this module, that creates `_FrontendCompiler` instance and
/// processes user input.
/// `compiler` is an optional parameter so it can be replaced with mocked
/// version for testing.
Future<int> starter(List<String> args, {CompilerInterface compiler}) async {
  final ArgResults options = _argParser.parse(args);
  if (options['train']) {
    return 0;
  }

  compiler ??= new _FrontendCompiler();

  if (options.rest.isNotEmpty) {
    await compiler.compile(options.rest[0], options);
    return 0;
  }

  _State state = _State.READY_FOR_INSTRUCTION;
  String boundaryKey;
  print('Frontend server is ready.');
  stdin
      .transform(UTF8.decoder)
      .transform(new LineSplitter())
      .listen((String string) async {
    switch (state) {
      case _State.READY_FOR_INSTRUCTION:
        if (string.startsWith('compile ')) {
          final String filename = string.substring('compile '.length);
          await compiler.compile(filename, options);
        } else if (string.startsWith('recompile ')) {
          boundaryKey = string.substring('recompile '.length);
          state = _State.RECOMPILE_LIST;
        } else if (string == 'accept') {
          compiler.acceptLastDelta();
        } else if (string == 'reject') {
          compiler.rejectLastDelta();
        } else if (string == 'quit') {
          exit(0);
        }
        break;
      case _State.RECOMPILE_LIST:
        if (string == boundaryKey) {
          compiler.recompileDelta();
          state = _State.READY_FOR_INSTRUCTION;
        } else {
          compiler.invalidate(Uri.base.resolve(string));
        }
        break;
    }
  });
  return 0;
}
