// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart' as ui;

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('CanvasKit text', () {
    setUpCanvasKitTest();

    test("doesn't crash when using static text styles", () {
      ui.ParagraphBuilder builder =
          ui.ParagraphBuilder(StaticTestStyles.staticParagraphStyle);
      builder.pushStyle(StaticTestStyles.staticTestStyle);
      builder.addText('test');
      builder.pushStyle(StaticTestStyles.staticTestStyle);
      final ui.Paragraph paragraph = builder.build();

      ui.ParagraphBuilder builder2 =
          ui.ParagraphBuilder(StaticTestStyles.staticParagraphStyle);
      builder2.pushStyle(StaticTestStyles.staticTestStyle);
      builder2.addText('test');
      builder2.pushStyle(StaticTestStyles.staticTestStyle);
      final ui.Paragraph paragraph2 = builder2.build();
    });
  }, skip: isIosSafari);
}

class StaticTestStyles {
  static ui.ParagraphStyle staticParagraphStyle =
      ui.ParagraphStyle(fontSize: 16);

  static ui.TextStyle staticTestStyle =
      ui.TextStyle(fontSize: 16, color: staticTestBlue);

  static ui.Color staticTestBlue = ui.Color.fromARGB(255, 0, 0, 255);
}
