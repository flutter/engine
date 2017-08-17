import 'package:args/src/arg_results.dart';
import 'package:frontend_server/server.dart';
import 'package:test/test.dart';

class MockedCompiler implements CompilerInterface {
  void Function(String, ArgResults) compileCallback;
  void Function(Uri) invalidateCallback;
  void Function() recompileDeltaCallback;
  void Function() acceptLastDeltaCallback;
  void Function() rejectLastDeltaCallback;

  MockedCompiler({this.compileCallback, this.recompileDeltaCallback,
    this.acceptLastDeltaCallback, this.rejectLastDeltaCallback,
    this.invalidateCallback});

  @override
  compile(String filename, ArgResults options) =>
      compileCallback != null ? compileCallback(filename, options) : null;

  @override
  invalidate(Uri uri) =>
      invalidateCallback != null ? invalidateCallback(uri) : null;

  @override
  recompileDelta() =>
      recompileDeltaCallback != null ?
      recompileDeltaCallback() : null;

  @override
  acceptLastDelta() =>
      acceptLastDeltaCallback != null ? acceptLastDeltaCallback() : null;

  @override
  rejectLastDelta() =>
      rejectLastDeltaCallback != null ? rejectLastDeltaCallback() : null;
}

main() async {
  group('basic', () {
    test('train completes', () async {
      expect(await starter(['--train']), equals(0));
    });
  });

  group('batch compile', () {
    test('compile from command line', () async {
      var args = [
        'server.dart',
        '--sdk-root', 'sdkroot'
      ];
      int exitcode = await starter(args,
          compiler: new MockedCompiler(
              compileCallback: (String filename, ArgResults options) {
                expect(filename, equals('server.dart'));
                expect(options['sdk-root'], equals('sdkroot'));
              }
          ));
      expect(exitcode, equals(0));
    });
  });
}
