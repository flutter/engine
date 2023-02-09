// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import '../../engine.dart';

/// Provides surfaces for painting.
class SurfaceFactory {
  static final SurfaceFactory instance = SurfaceFactory();

  /// The base surface to paint on. This is the default surface which will be
  /// painted to.
  final Surface baseSurface = Surface();

  /// A surface used specifically for `Picture.toImage` when software rendering
  /// is supported.
  late final Surface pictureToImageSurface = Surface();
}
