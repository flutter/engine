import 'package:args/src/arg_results.dart';
import 'package:frontend_server/server.dart';
import 'package:test/test.dart';

class MockedCompiler implements CompilerInterface {
  var compileCallback;
  var recompileDeltaCallback;
  var acceptLastDeltaCallback;
  var rejectLastDeltaCallback;
  var invalideCallback;

  MockedCompiler({this.compileCallback, this.recompileDeltaCallback,
    this.acceptLastDeltaCallback, this.rejectLastDeltaCallback,
    this.invalideCallback});

  @override
  compile(String filename, ArgResults options) =>
      compileCallback != null ? compileCallback(filename, options) : null;

  @override
  invalidate(Uri uri) =>
      invalideCallback != null ? invalideCallback(uri) : null;

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
        '--sdk-root', 'sdkroot',
        '--platform-kernel-dill', 'platform.dill'
      ];
      int exitcode = await starter(args,
          compiler: new MockedCompiler(
              compileCallback: (String filename, ArgResults options) {
                expect(filename, equals('server.dart'));
                expect(options['sdk-root'], equals('sdkroot'));
                expect(
                    options['platform-kernel-dill'], equals('platform.dill'));
              }
          ));
      expect(exitcode, equals(0));
    });
  });
}
