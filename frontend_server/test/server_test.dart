import 'dart:async';

import 'package:args/src/arg_results.dart';
import 'package:frontend_server/server.dart';
import 'package:test/test.dart';

class _MockedCompiler implements CompilerInterface {
  _MockedCompiler(
      {this.compileCallback,
      this.recompileDeltaCallback,
      this.acceptLastDeltaCallback,
      this.rejectLastDeltaCallback,
      this.invalidateCallback});

  Future<Null> Function(String, ArgResults) compileCallback;
  Future<Null> Function() recompileDeltaCallback;
  void Function() acceptLastDeltaCallback;
  void Function() rejectLastDeltaCallback;
  void Function(Uri) invalidateCallback;

  @override
  Future<Null> compile(String filename, ArgResults options) {
    return compileCallback != null ? compileCallback(filename, options) : null;
  }

  @override
  void invalidate(Uri uri) {
    if (invalidateCallback != null)
      invalidateCallback(uri);
  }

  @override
  Future<Null> recompileDelta() {
    return recompileDeltaCallback != null ? recompileDeltaCallback() : null;
  }

  @override
  void acceptLastDelta() {
    if (acceptLastDeltaCallback != null)
      acceptLastDeltaCallback();
  }

  @override
  void rejectLastDelta() {
    if (rejectLastDeltaCallback != null)
      rejectLastDeltaCallback();
  }
}

Future<int> main() async {
  group('basic', () {
    test('train completes', () async {
      expect(await starter(<String>['--train']), equals(0));
    });
  });

  group('batch compile', () {
    test('compile from command line', () async {
      final List<String> args = <String>[
        'server.dart',
        '--sdk-root',
        'sdkroot'
      ];
      final int exitcode = await starter(args, compiler: new _MockedCompiler(
          compileCallback: (String filename, ArgResults options) {
        expect(filename, equals('server.dart'));
        expect(options['sdk-root'], equals('sdkroot'));
      }));
      expect(exitcode, equals(0));
    });
  });
  return 0;
}
