import 'package:test/test.dart';

import 'package:frontend_server/server.dart';

main() async {
  group('basic', () {
    test('init', () async {
      expect(await starter(['--train']), equals(0));
    });
  });
}
