// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ui' as ui;

import 'package:test/test.dart';

void main() {

  group('path equality', () {
    test('empty paths', () async {
      ui.Path path1 = new ui.Path();
      ui.Path path2 = new ui.Path();
      expect(path1, path2);
    });

    test('empty does not equal non empty', () async {
      ui.Path path1 = new ui.Path();
      ui.Path path2 = new ui.Path();
      path2.addRRect(new ui.RRect.fromLTRBR(0.0, 0.0, 10.0, 10.0, const ui.Radius.circular(2.0)));
      expect(path1, isNot(path2));
    });

    test('same contents', () async {
      ui.Path path1 = new ui.Path();
      ui.Path path2 = new ui.Path();
      path2.addRRect(new ui.RRect.fromLTRBR(0.0, 0.0, 10.0, 10.0, const ui.Radius.circular(2.0)));
      path1.addRRect(new ui.RRect.fromLTRBR(0.0, 0.0, 10.0, 10.0, const ui.Radius.circular(2.0)));
      expect(path1, path2);
    });

    test('different contents', () async {
      ui.Path path1 = new ui.Path();
      ui.Path path2 = new ui.Path();
      path2.addRRect(new ui.RRect.fromLTRBR(0.0, 0.0, 10.0, 10.0, const ui.Radius.circular(2.0)));
      path1.addRRect(new ui.RRect.fromLTRBR(0.0, 0.0, 5.0, 10.0, const ui.Radius.circular(2.0)));
      expect(path1, isNot(path2));
    });
  });
}
