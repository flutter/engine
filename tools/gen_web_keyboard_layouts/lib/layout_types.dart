// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart' show immutable;

enum LayoutPlatform {
  win,
  linux,
  darwin,
}

@immutable
class LayoutEntry {
  LayoutEntry(this.printables, this.deadMasks)
    : assert(printables.length == 4);

  final List<String> printables;
  final int deadMasks;

  static final LayoutEntry empty = LayoutEntry(
    const <String>['', '', '', ''], 0xf);
}

@immutable
class Layout {
  const Layout(this.language, this.platform, this.entries);

  final String language;
  final LayoutPlatform platform;
  final Map<String, LayoutEntry> entries;
}

@immutable
class LayoutStore {
  const LayoutStore(this.layouts);

  final List<Layout> layouts;
}
