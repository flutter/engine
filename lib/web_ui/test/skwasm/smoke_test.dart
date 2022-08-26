@TestOn('chrome || safari || firefox')

import 'dart:async';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine/renderer.dart';
import 'package:ui/src/engine/skwasm/skwasm_stub/renderer.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  group('Skwasm tests', () {
    test('Skwasm renderer throws', () {
      expect(renderer, isA<SkwasmRenderer>());
      expect(() {
        renderer.initialize();
      }, throwsUnimplementedError);
    });
  });
}
