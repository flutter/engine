// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// A cache of Skia objects whose memory Flutter manages.
///
/// When using Skia, Flutter creates Skia objects which are allocated in
/// WASM memory and which must be explicitly deleted. In the case of Flutter
/// mobile, the Skia objects are wrapped by a C++ class which is destroyed
/// when the associated Dart object is garbage collected.
///
/// On the web, we cannot tell when a Dart object is garbage collected, so
/// we must use other strategies to know when to delete a Skia object. Some
/// objects, like [ui.Paint], can safely delete their associated Skia object
/// because they can always recreate the Skia object from data stored in the
/// Dart object. Other objects, like [ui.Picture], can be serialized to a
/// JS-managed data structure when they are deleted so that when the associated
/// object is garbage collected, so is the serialized data.
class SkiaObjectCache {
  final int maximumSize;

  final DoubleLinkedQueue<SkiaObject> _itemQueue;
  final Map<SkiaObject, DoubleLinkedQueueEntry<SkiaObject>> _itemMap;

  SkiaObjectCache(this.maximumSize)
      : _itemQueue = DoubleLinkedQueue<SkiaObject>(),
        _itemMap = <SkiaObject, DoubleLinkedQueueEntry<SkiaObject>>{};

  int get length => _itemQueue.length;

  bool contains(SkiaObject object) {
    return _itemQueue.contains(object);
  }

  /// Adds [object] to the cache.
  ///
  /// If adding [object] causes the total size of the cache to exceed
  /// [maximumSize], then the least recently used half of the cache
  /// will be deleted.
  void add(SkiaObject object) {
    _itemQueue.addFirst(object);
    DoubleLinkedQueueEntry<SkiaObject> item = _itemQueue.firstEntry()!;
    _itemMap[object] = item;

    if (_itemQueue.length > maximumSize) {
      SkiaObjects.markCacheForResize(this);
    }
  }

  /// Records that [object] was used in the most recent frame.
  void markUsed(SkiaObject object) {
    DoubleLinkedQueueEntry<SkiaObject> item = _itemMap[object]!;
    item.remove();
    _itemQueue.addFirst(object);
    DoubleLinkedQueueEntry<SkiaObject> newItem = _itemQueue.firstEntry()!;
    _itemMap[object] = newItem;
  }

  /// Deletes the least recently used half of this cache.
  void resize() {
    final int itemsToDelete = maximumSize ~/ 2;
    for (int i = 0; i < itemsToDelete; i++) {
      final SkiaObject oldObject = _itemQueue.removeLast();
      _itemMap.remove(oldObject);
      oldObject.delete();
    }
  }
}

/// An object backed by a [js.JsObject] mapped onto a Skia C++ object in the
/// WebAssembly heap.
///
/// These objects are automatically deleted when no longer used.
abstract class SkiaObject {
  /// The JavaScript object that's mapped onto a Skia C++ object in the WebAssembly heap.
  js.JsObject? get skiaObject;

  /// Deletes the associated C++ object from the WebAssembly heap.
  void delete();
}

/// A [SkiaObject] that can resurrect its C++ counterpart.
///
/// Because there is no feedback from JavaScript's GC (no destructors or
/// finalizers), we pessimistically delete the underlying C++ object before
/// the Dart object is garbage-collected. The current algorithm deletes objects
/// at the end of every frame. This allows reusing the C++ objects within the
/// frame. In the future we may add smarter strategies that will allow us to
/// reuse C++ objects across frames.
///
/// The lifecycle of a C++ object is as follows:
///
/// - Create default: when instantiating a C++ object for a Dart object for the
///   first time, the C++ object is populated with default data (the defaults are
///   defined by Flutter; Skia defaults are corrected if necessary). The
///   default object is created by [createDefault].
/// - Zero or more cycles of delete + resurrect: when a Dart object is reused
///   after its C++ object is deleted we create a new C++ object populated with
///   data from the current state of the Dart object. This is done using the
///   [resurrect] method.
/// - Final delete: if a Dart object is never reused, it is GC'd after its
///   underlying C++ object is deleted. This is implemented by [SkiaObjects].
abstract class ResurrectableSkiaObject extends SkiaObject {
  ResurrectableSkiaObject() {
    _skiaObject = createDefault();
    SkiaObjects.manageResurrectable(this);
  }

  @override
  js.JsObject? get skiaObject {
    if (_skiaObject == null) {
      _skiaObject = resurrect();
      SkiaObjects.manageResurrectable(this);
    }
    return _skiaObject;
  }

  /// Do not use this field outside this class. Use [skiaObject] instead.
  js.JsObject? _skiaObject;

  /// Instantiates a new Skia-backed JavaScript object containing default
  /// values.
  ///
  /// The object is expected to represent Flutter's defaults. If Skia uses
  /// different defaults from those used by Flutter, this method is expected
  /// initialize the object to Flutter's defaults.
  js.JsObject createDefault();

  /// Creates a new Skia-backed JavaScript object containing data representing
  /// the current state of the Dart object.
  js.JsObject resurrect();

  @override
  void delete() {
    _skiaObject!.callMethod('delete');
    _skiaObject = null;
  }
}

/// A [SkiaObject] which is deleted once and cannot be used again.
class OneShotSkiaObject extends SkiaObject {
  js.JsObject? _skiaObject;

  OneShotSkiaObject(this._skiaObject) {
    SkiaObjects.manageOneShot(this);
  }

  @override
  js.JsObject? get skiaObject {
    if (_skiaObject == null) {
      throw StateError('Attempting to use a Skia object that has been freed.');
    }
    SkiaObjects.objectCache.markUsed(this);
    return _skiaObject;
  }

  @override
  void delete() {
    _skiaObject!.callMethod('delete');
    _skiaObject = null;
  }
}

/// Singleton that manages the lifecycles of [SkiaObject] instances.
class SkiaObjects {
  // TODO(yjbanov): some sort of LRU strategy would allow us to reuse objects
  //                beyond a single frame.
  @visibleForTesting
  static final List<ResurrectableSkiaObject> resurrectableObjects =
      <ResurrectableSkiaObject>[];

  @visibleForTesting
  static int maximumCacheSize = 8192;

  @visibleForTesting
  static final SkiaObjectCache objectCache = SkiaObjectCache(maximumCacheSize);

  @visibleForTesting
  static final List<SkiaObjectCache> cachesToResize = <SkiaObjectCache>[];

  static bool _addedCleanupCallback = false;

  @visibleForTesting
  static void registerCleanupCallback() {
    if (_addedCleanupCallback) {
      return;
    }
    window.rasterizer!.addPostFrameCallback(postFrameCleanUp);
    _addedCleanupCallback = true;
  }

  /// Starts managing the lifecycle of resurrectable [object].
  ///
  /// These can safely be deleted at any time.
  static void manageResurrectable(ResurrectableSkiaObject object) {
    registerCleanupCallback();
    resurrectableObjects.add(object);
  }

  /// Starts managing the lifecycle of a one-shot [object].
  ///
  /// We should avoid deleting these whenever we can, since we won't
  /// be able to resurrect them.
  static void manageOneShot(OneShotSkiaObject object) {
    registerCleanupCallback();
    objectCache.add(object);
  }

  /// Marks that [cache] has overflown its maximum size and show be resized.
  static void markCacheForResize(SkiaObjectCache cache) {
    registerCleanupCallback();
    if (cachesToResize.contains(cache)) {
      return;
    }
    cachesToResize.add(cache);
  }

  /// Cleans up managed Skia memory.
  static void postFrameCleanUp() {
    if (resurrectableObjects.isEmpty && cachesToResize.isEmpty) {
      return;
    }

    for (int i = 0; i < resurrectableObjects.length; i++) {
      final SkiaObject object = resurrectableObjects[i];
      object.delete();
    }
    resurrectableObjects.clear();

    for (int i = 0; i < cachesToResize.length; i++) {
      final SkiaObjectCache cache = cachesToResize[i];
      cache.resize();
    }
    cachesToResize.clear();
  }
}
