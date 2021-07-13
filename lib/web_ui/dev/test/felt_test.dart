import 'package:test/test.dart';
import 'dart:io';
import 'package:path/path.dart' as p;

void main() {
  test('felt.dart --usage exits 0', () async {
    var dartExecutable = Platform.executable;
    var testPath = Platform.script.toFilePath();
    var feltPath = p.join(p.dirname(p.dirname(testPath)), 'felt.dart');
    var result = await Process.run(dartExecutable, [feltPath, '--help']);
    expect(result.exitCode, equals(0));
    expect(result.stdout, contains('usage'));
  });
}
