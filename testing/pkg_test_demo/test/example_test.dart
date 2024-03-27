import 'package:test/test.dart';

void main() {
  test('String.split() splits the string on the delimiter', () {
    const String string = 'foo,bar,baz';
    expect(string.split(','), equals(['foo', 'bar', 'baz']));
  });

  test('String.trim() removes surrounding whitespace', () {
    const String string = '  foo ';
    expect(string.trim(), equals('foo'));
  });
}
