// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

abstract class EngineCodec implements ui.Codec {

  int get width => -1;
  int get height => -1;
}
