// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:math' as math show max;

import 'package:meta/meta.dart';

import '../../engine.dart';

/// Caches snapshot surfaces used to overlay platform views.
class SnapshotSurfaceFactory {
  SnapshotSurfaceFactory(int maximumSurfaces)
      : maximumSurfaces = math.max(maximumSurfaces, 1) {
    if (assertionsEnabled) {
      if (maximumSurfaces < 1) {
        printWarning('Attempted to create a $SnapshotSurfaceFactory with '
            '$maximumSurfaces maximum surfaces. At least 1 surface is required '
            'for rendering.');
      }
      registerHotRestartListener(debugClear);
    }
  }

  /// The lazy-initialized singleton surface factory.
  ///
  /// [debugClear] causes this singleton to be reinitialized.
  static SnapshotSurfaceFactory get instance => _instance ??=
      SnapshotSurfaceFactory(configuration.canvasKitMaximumSurfaces);

  /// Returns the raw (potentially uninitialized) value of the singleton.
  ///
  /// Useful in tests for checking the lifecycle of this class.
  static SnapshotSurfaceFactory? get debugUninitializedInstance => _instance;

  // Override the current instance with a new one.
  //
  // This should only be used in tests.
  static void debugSetInstance(SnapshotSurfaceFactory newInstance) {
    _instance = newInstance;
  }

  static SnapshotSurfaceFactory? _instance;

  /// The maximum number of surfaces which can be live at once.
  final int maximumSurfaces;

  /// The maximum number of assignable overlays.
  ///
  /// This is just `maximumSurfaces - 1` (the maximum number of surfaces minus
  /// the required base surface).
  int get maximumOverlays => maximumSurfaces - 1;

  /// Surfaces created by this factory which are currently in use.
  final List<SnapshotSurface> _liveSurfaces = <SnapshotSurface>[];

  /// Surfaces created by this factory which are no longer in use. These can be
  /// reused.
  final List<SnapshotSurface> _cache = <SnapshotSurface>[];

  /// The number of surfaces which have been created by this factory.
  int get _surfaceCount => _liveSurfaces.length + _cache.length + 1;

  /// The number of available overlay surfaces.
  ///
  /// This does not include the base surface.
  int get numAvailableOverlays => maximumOverlays - _liveSurfaces.length;

  /// The number of surfaces created by this factory. Used for testing.
  @visibleForTesting
  int get debugSurfaceCount => _surfaceCount;

  /// Returns the number of cached surfaces.
  ///
  /// Useful in tests.
  int get debugCacheSize => _cache.length;

  /// Gets an overlay surface from the cache or creates a new one if it wouldn't
  /// exceed the maximum. If there are no available surfaces, returns `null`.
  SnapshotSurface? getSurface() {
    if (_cache.isNotEmpty) {
      final SnapshotSurface surface = _cache.removeLast();
      _liveSurfaces.add(surface);
      return surface;
    } else if (debugSurfaceCount < maximumSurfaces) {
      final SnapshotSurface surface = SnapshotSurface();
      _liveSurfaces.add(surface);
      return surface;
    } else {
      return null;
    }
  }

  /// Releases all surfaces so they can be reused in the next frame.
  ///
  /// If a released surface is in the DOM, it is not removed. This allows the
  /// engine to release the surfaces at the end of the frame so they are ready
  /// to be used in the next frame, but still used for painting in the current
  /// frame.
  void releaseSurfaces() {
    _cache.addAll(_liveSurfaces);
    _liveSurfaces.clear();
  }

  /// Removes all snapshot surfaces from the DOM.
  ///
  /// This is called at the beginning of the frame to prepare for painting into
  /// the new surfaces.
  void removeSurfacesFromDom() {
    _cache.forEach(_removeFromDom);
  }

  /// Removes [surface] from the DOM.
  void _removeFromDom(SnapshotSurface surface) {
    surface.htmlElement.remove();
  }

  /// Signals that a surface is no longer being used. It can be reused.
  void releaseSurface(SnapshotSurface surface) {
    assert(
        _liveSurfaces.contains(surface),
        'Attempting to release a Surface which '
        'was not created by this factory');
    surface.htmlElement.remove();
    _liveSurfaces.remove(surface);
    _cache.add(surface);
  }

  /// Returns [true] if [surface] is currently being used to paint content.
  ///
  /// The base surface always counts as live.
  ///
  /// If a surface is not live, then it must be in the cache and ready to be
  /// reused.
  bool isLive(SnapshotSurface surface) {
    if (_liveSurfaces.contains(surface)) {
      return true;
    }
    assert(_cache.contains(surface));
    return false;
  }

  /// Dispose all snapshot surfaces created by this factory. Used in tests.
  void debugClear() {
    for (final SnapshotSurface surface in _cache) {
      surface.dispose();
    }
    for (final SnapshotSurface surface in _liveSurfaces) {
      surface.dispose();
    }
    _liveSurfaces.clear();
    _cache.clear();
    _instance = null;
  }
}
