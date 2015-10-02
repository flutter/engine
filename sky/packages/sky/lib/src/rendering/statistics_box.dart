// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:sky/src/rendering/box.dart';
import 'package:sky/src/rendering/object.dart';

class StatisticsBox extends RenderBox {

  StatisticsBox(this.optionsMask);

  final int optionsMask;

  bool get hasLayer => false;

  void performLayout() {
    size = constraints.biggest;
  }

  void paint(PaintingContext context, Offset offset) {
    context.paintStatistics(optionsMask, offset, size);
  }
}
