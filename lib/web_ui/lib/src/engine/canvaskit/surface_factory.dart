// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'embedded_views.dart';
import 'surface.dart';

/// Caches surfaces used to overlay platform views.
class SurfaceFactory {
  /// The cache singleton.
  static final SurfaceFactory instance =
      SurfaceFactory(HtmlViewEmbedder.maximumOverlaySurfaces);

  SurfaceFactory(this.maximumSize);

  /// The cache will not grow beyond this size.
  final int maximumSize;

  /// The number of surfaces which have been created by this cache.
  ///
  /// Calls to [createOverlay] will only return non-null if this cache hasn't
  int _surfacesCreated = 0;

  /// Cached surfaces, available for reuse.
  final List<Surface> _cache = <Surface>[];

  /// Returns the list of cached surfaces.
  ///
  /// Useful in tests.
  List<Surface> get debugCachedSurfaces => _cache;

  /// Creates a new overlay, unless this cache has already created [maximumSize]
  /// overlays.
  Surface? createOverlay(HtmlViewEmbedder viewEmbedder) {
    if (_surfacesCreated < maximumSize) {
      _surfacesCreated++;
      return Surface(viewEmbedder);
    }
    return null;
  }

  /// Reserves an overlay from the cache.
  ///
  /// If the maximum number of overlays have already been reserved, this returns
  /// [null].
  Surface? reserveOverlay(HtmlViewEmbedder viewEmbedder) {
    if (_cache.isEmpty) {
      return null;
    }
    return _cache.removeLast();
  }

  /// Returns an overlay back to the cache.
  ///
  /// If the cache is full, the overlay is deleted.
  void releaseOverlay(Surface overlay) {
    overlay.htmlElement.remove();
    if (_cache.length < maximumSize) {
      _cache.add(overlay);
    } else {
      _surfacesCreated--;
      overlay.dispose();
    }
  }

  int get debugLength => _cache.length;

  void debugClear() {
    for (final Surface overlay in _cache) {
      overlay.dispose();
    }
    _cache.clear();
    _surfacesCreated = 0;
  }
}
