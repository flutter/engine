// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../../engine.dart' show platformViewManager;
import '../configuration.dart';
import '../dom.dart';
import '../html/path_to_svg_clip.dart';
import '../platform_views/slots.dart';
import '../svg.dart';
import '../util.dart';
import '../vector_math.dart';
import '../window.dart';
import 'canvas.dart';
import 'initialization.dart';
import 'path.dart';
import 'picture_recorder.dart';
import 'surface.dart';
import 'surface_factory.dart';

/// This composites HTML views into the [ui.Scene].
class HtmlViewEmbedder {
  /// The [HtmlViewEmbedder] singleton.
  static HtmlViewEmbedder instance = HtmlViewEmbedder._();

  HtmlViewEmbedder._();

  /// Force the view embedder to disable overlays.
  ///
  /// This should never be used outside of tests.
  static set debugDisableOverlays(bool disable) {
    final SurfaceFactory? instance = SurfaceFactory.debugUninitializedInstance;
    if (instance != null) {
      instance.releaseSurfaces();
      instance.removeSurfacesFromDom();
      instance.debugClear();
    }
    if (disable) {
      // If we are disabling overlays then get the current [SurfaceFactory]
      // instance, clear it, and overwrite it with a new instance with only
      // two surfaces (base and backup).
      SurfaceFactory.debugSetInstance(SurfaceFactory(1));
    } else {
      // If we are re-enabling overlays then replace the current
      // [SurfaceFactory]instance with one with
      // [configuration.canvasKitMaximumSurfaces] overlays.
      SurfaceFactory.debugSetInstance(
          SurfaceFactory(configuration.canvasKitMaximumSurfaces));
    }
  }

  /// Whether or not we have seen a visible platform view in this frame yet.
  bool _seenFirstVisibleViewInPreroll = false;

  /// Whether or not we have seen a visible platform view in this frame yet.
  bool _seenFirstVisibleView = false;

  /// Picture recorders which were created during the preroll phase.
  ///
  /// These picture recorders will be "claimed" in the paint phase by platform
  /// views being composited into the scene.
  final List<CkPictureRecorder> _pictureRecordersCreatedDuringPreroll =
      <CkPictureRecorder>[];

  /// Picture recorders which were actually used in the paint phase.
  ///
  /// This is a subset of [_pictureRecordersCreatedDuringPreroll].
  final List<CkPictureRecorder> _pictureRecorders = <CkPictureRecorder>[];

  /// The most recent composition parameters for a given view id.
  ///
  /// If we receive a request to composite a view, but the composition
  /// parameters haven't changed, we can avoid having to recompute the
  /// element stack that correctly composites the view into the scene.
  final Map<int, EmbeddedViewParams> _currentCompositionParams =
      <int, EmbeddedViewParams>{};

  /// The clip chain for a view Id.
  ///
  /// This contains:
  /// * The root view in the stack of mutator elements for the view id.
  /// * The slot view in the stack (what shows the actual platform view contents).
  /// * The number of clipping elements used last time the view was composited.
  final Map<int, ViewClipChain> _viewClipChains = <int, ViewClipChain>{};

  /// Surfaces used to draw on top of platform views, keyed by platform view ID.
  ///
  /// These surfaces are cached in the [OverlayCache] and reused.
  final Map<int, Surface> _overlays = <int, Surface>{};

  /// The views that need to be recomposited into the scene on the next frame.
  final Set<int> _viewsToRecomposite = <int>{};

  /// The list of view ids that should be composited, in order.
  List<int> _compositionOrder = <int>[];

  /// The number of platform views in this frame which are visible.
  ///
  /// These platform views will require overlays.
  int _visibleViewCount = 0;

  /// The most recent composition order.
  List<int> _activeCompositionOrder = <int>[];

  /// The size of the frame, in physical pixels.
  ui.Size _frameSize = ui.window.physicalSize;

  set frameSize(ui.Size size) {
    _frameSize = size;
  }

  /// Returns a list of canvases which will be overlaid on top of the "base"
  /// canvas after a platform view is composited into the scene.
  ///
  /// The engine asks for the overlay canvases immediately before the paint
  /// phase, after the preroll phase. In the preroll phase we must be
  /// conservative and assume that every platform view which is prerolled is
  /// also composited, and therefore requires an overlay canvas. However, not
  /// every platform view which is prerolled ends up being composited (it may be
  /// clipped out and not actually drawn). This means that we may end up
  /// overallocating canvases. This isn't a problem in practice, however, as
  /// unused recording canvases are simply deleted at the end of the frame.
  List<CkCanvas> getOverlayCanvases() {
    final List<CkCanvas> overlayCanvases = _pictureRecordersCreatedDuringPreroll
        .map((CkPictureRecorder r) => r.recordingCanvas!)
        .toList();
    return overlayCanvases;
  }

  void prerollCompositeEmbeddedView(int viewId, EmbeddedViewParams params) {
    final bool hasAvailableOverlay =
        _pictureRecordersCreatedDuringPreroll.length <
            SurfaceFactory.instance.maximumOverlays;
    // We need an overlay for the first platform view no matter what. The first
    // visible platform view doesn't need to create a new one if we already
    // created one.
    final bool needNewOverlay = (platformViewManager.isVisible(viewId) &&
            _seenFirstVisibleViewInPreroll) ||
        _pictureRecordersCreatedDuringPreroll.isEmpty;
    if (platformViewManager.isVisible(viewId)) {
      _seenFirstVisibleViewInPreroll = true;
    }
    if (needNewOverlay && hasAvailableOverlay) {
      final CkPictureRecorder pictureRecorder = CkPictureRecorder();
      pictureRecorder.beginRecording(ui.Offset.zero & _frameSize);
      pictureRecorder.recordingCanvas!.clear(const ui.Color(0x00000000));
      _pictureRecordersCreatedDuringPreroll.add(pictureRecorder);
    }

    // Do nothing if the params didn't change.
    if (_currentCompositionParams[viewId] == params) {
      // If the view was prerolled but not composited, then it needs to be
      // recomposited.
      if (!_activeCompositionOrder.contains(viewId)) {
        _viewsToRecomposite.add(viewId);
      }
      return;
    }
    _currentCompositionParams[viewId] = params;
    _viewsToRecomposite.add(viewId);
  }

  /// Prepares to composite [viewId].
  ///
  /// If this returns a [CkCanvas], then that canvas should be the new leaf
  /// node. Otherwise, keep the same leaf node.
  CkCanvas? compositeEmbeddedView(int viewId) {
    final int overlayIndex = _visibleViewCount;
    _compositionOrder.add(viewId);
    if (platformViewManager.isVisible(viewId) || _pictureRecorders.isEmpty) {
      _visibleViewCount++;
    }
    // We need a new overlay if this is a visible view or if we don't have one yet.
    final bool needNewOverlay =
        (platformViewManager.isVisible(viewId) && _seenFirstVisibleView) ||
            _pictureRecorders.isEmpty;
    if (platformViewManager.isVisible(viewId)) {
      _seenFirstVisibleView = true;
    }
    CkPictureRecorder? recorderToUseForRendering;
    if (needNewOverlay) {
      if (overlayIndex < _pictureRecordersCreatedDuringPreroll.length) {
        recorderToUseForRendering =
            _pictureRecordersCreatedDuringPreroll[overlayIndex];
        _pictureRecorders.add(recorderToUseForRendering);
      }
    }

    if (_viewsToRecomposite.contains(viewId)) {
      _compositeWithParams(viewId, _currentCompositionParams[viewId]!);
      _viewsToRecomposite.remove(viewId);
    }
    return recorderToUseForRendering?.recordingCanvas;
  }

  void _compositeWithParams(int viewId, EmbeddedViewParams params) {
    // If we haven't seen this viewId yet, cache it for clips/transforms.
    final ViewClipChain clipChain = _viewClipChains.putIfAbsent(viewId, () {
      return ViewClipChain(view: createPlatformViewSlot(viewId));
    });

    final DomElement slot = clipChain.slot;

    // See `apply()` in the PersistedPlatformView class for the HTML version
    // of this code.
    slot.style
      ..width = '${params.size.width}px'
      ..height = '${params.size.height}px'
      ..position = 'absolute';

    // Recompute the position in the DOM of the `slot` element...
    final int currentClippingCount = _countClips(params.mutators);
    final int previousClippingCount = clipChain.clipCount;
    if (currentClippingCount != previousClippingCount) {
      final DomElement oldPlatformViewRoot = clipChain.root;
      final DomElement newPlatformViewRoot = _reconstructClipViewsChain(
        currentClippingCount,
        slot,
        oldPlatformViewRoot,
      );
      // Store the updated root element, and clip count
      clipChain.updateClipChain(
        root: newPlatformViewRoot,
        clipCount: currentClippingCount,
      );
    }

    // Apply mutators to the slot
    _applyMutators(params, slot, viewId);
  }

  int _countClips(MutatorsStack mutators) {
    int clipCount = 0;
    for (final Mutator mutator in mutators) {
      if (mutator.isClipType) {
        clipCount++;
      }
    }
    return clipCount;
  }

  DomElement _reconstructClipViewsChain(
    int numClips,
    DomElement platformView,
    DomElement headClipView,
  ) {
    DomNode? headClipViewNextSibling;
    bool headClipViewWasAttached = false;
    if (headClipView.parentNode != null) {
      headClipViewWasAttached = true;
      headClipViewNextSibling = headClipView.nextSibling;
      headClipView.remove();
    }
    DomElement head = platformView;
    int clipIndex = 0;
    // Re-use as much existing clip views as needed.
    while (head != headClipView && clipIndex < numClips) {
      head = head.parent!;
      clipIndex++;
    }
    // If there weren't enough existing clip views, add more.
    while (clipIndex < numClips) {
      final DomElement clippingView = createDomElement('flt-clip');
      clippingView.append(head);
      head = clippingView;
      clipIndex++;
    }
    head.remove();

    // If the chain was previously attached, attach it to the same position.
    if (headClipViewWasAttached) {
      skiaSceneHost!.insertBefore(head, headClipViewNextSibling);
    }
    return head;
  }

  /// Clean up the old SVG clip definitions, as this platform view is about to
  /// be recomposited.
  void _cleanUpClipDefs(int viewId) {
    if (_svgClipDefs.containsKey(viewId)) {
      final DomElement clipDefs = _svgPathDefs!.querySelector('#sk_path_defs')!;
      final List<DomElement> nodesToRemove = <DomElement>[];
      final Set<String> oldDefs = _svgClipDefs[viewId]!;
      for (final DomElement child in clipDefs.children) {
        if (oldDefs.contains(child.id)) {
          nodesToRemove.add(child);
        }
      }
      for (final DomElement node in nodesToRemove) {
        node.remove();
      }
      _svgClipDefs[viewId]!.clear();
    }
  }

  void _applyMutators(
      EmbeddedViewParams params, DomElement embeddedView, int viewId) {
    final MutatorsStack mutators = params.mutators;
    DomElement head = embeddedView;
    Matrix4 headTransform = params.offset == ui.Offset.zero
        ? Matrix4.identity()
        : Matrix4.translationValues(params.offset.dx, params.offset.dy, 0);
    double embeddedOpacity = 1.0;
    _resetAnchor(head);
    _cleanUpClipDefs(viewId);

    for (final Mutator mutator in mutators) {
      switch (mutator.type) {
        case MutatorType.transform:
          headTransform = mutator.matrix!.multiplied(headTransform);
          head.style.transform =
              float64ListToCssTransform(headTransform.storage);
          break;
        case MutatorType.clipRect:
        case MutatorType.clipRRect:
        case MutatorType.clipPath:
          final DomElement clipView = head.parent!;
          clipView.style.clip = '';
          clipView.style.clipPath = '';
          headTransform = Matrix4.identity();
          clipView.style.transform = '';
          if (mutator.rect != null) {
            final ui.Rect rect = mutator.rect!;
            clipView.style.clip = 'rect(${rect.top}px, ${rect.right}px, '
                '${rect.bottom}px, ${rect.left}px)';
          } else if (mutator.rrect != null) {
            final CkPath path = CkPath();
            path.addRRect(mutator.rrect!);
            _ensureSvgPathDefs();
            final DomElement pathDefs =
                _svgPathDefs!.querySelector('#sk_path_defs')!;
            _clipPathCount += 1;
            final String clipId = 'svgClip$_clipPathCount';
            final SVGClipPathElement newClipPath = createSVGClipPathElement();
            newClipPath.id = clipId;
            newClipPath.append(
                createSVGPathElement()..setAttribute('d', path.toSvgString()!));

            pathDefs.append(newClipPath);
            // Store the id of the node instead of [newClipPath] directly. For
            // some reason, calling `newClipPath.remove()` doesn't remove it
            // from the DOM.
            _svgClipDefs.putIfAbsent(viewId, () => <String>{}).add(clipId);
            clipView.style.clipPath = 'url(#$clipId)';
          } else if (mutator.path != null) {
            final CkPath path = mutator.path! as CkPath;
            _ensureSvgPathDefs();
            final DomElement pathDefs =
                _svgPathDefs!.querySelector('#sk_path_defs')!;
            _clipPathCount += 1;
            final String clipId = 'svgClip$_clipPathCount';
            final SVGClipPathElement newClipPath = createSVGClipPathElement();
            newClipPath.id = clipId;
            newClipPath.append(
                createSVGPathElement()..setAttribute('d', path.toSvgString()!));
            pathDefs.append(newClipPath);
            // Store the id of the node instead of [newClipPath] directly. For
            // some reason, calling `newClipPath.remove()` doesn't remove it
            // from the DOM.
            _svgClipDefs.putIfAbsent(viewId, () => <String>{}).add(clipId);
            clipView.style.clipPath = 'url(#$clipId)';
          }
          _resetAnchor(clipView);
          head = clipView;
          break;
        case MutatorType.opacity:
          embeddedOpacity *= mutator.alphaFloat;
          break;
      }
    }

    embeddedView.style.opacity = embeddedOpacity.toString();

    // Reverse scale based on screen scale.
    //
    // HTML elements use logical (CSS) pixels, but we have been using physical
    // pixels, so scale down the head element to match the logical resolution.
    final double scale = window.devicePixelRatio;
    final double inverseScale = 1 / scale;
    final Matrix4 scaleMatrix =
        Matrix4.diagonal3Values(inverseScale, inverseScale, 1);
    headTransform = scaleMatrix.multiplied(headTransform);
    head.style.transform = float64ListToCssTransform(headTransform.storage);
  }

  /// Sets the transform origin to the top-left corner of the element.
  ///
  /// By default, the transform origin is the center of the element, but
  /// Flutter assumes the transform origin is the top-left point.
  void _resetAnchor(DomElement element) {
    element.style.transformOrigin = '0 0 0';
    element.style.position = 'absolute';
  }

  int _clipPathCount = 0;

  DomElement? _svgPathDefs;

  /// The nodes containing the SVG clip definitions needed to clip this view.
  Map<int, Set<String>> _svgClipDefs = <int, Set<String>>{};

  /// Ensures we add a container of SVG path defs to the DOM so they can
  /// be referred to in clip-path: url(#blah).
  void _ensureSvgPathDefs() {
    if (_svgPathDefs != null) {
      return;
    }
    _svgPathDefs = kSvgResourceHeader.clone(false) as SVGElement;
    _svgPathDefs!.append(createSVGDefsElement()..id = 'sk_path_defs');
    skiaSceneHost!.append(_svgPathDefs!);
  }

  void submitFrame() {
    final ViewListDiffResult? diffResult =
        (_activeCompositionOrder.isEmpty || _compositionOrder.isEmpty)
            ? null
            : diffViewList(_activeCompositionOrder, _compositionOrder);
    _updateOverlays(diffResult);
    assert(_pictureRecorders.length == _overlays.length);
    int pictureRecorderIndex = 0;

    for (int i = 0; i < _compositionOrder.length; i++) {
      final int viewId = _compositionOrder[i];
      if (_overlays[viewId] != null) {
        final SurfaceFrame frame = _overlays[viewId]!.acquireFrame(_frameSize);
        final CkCanvas canvas = frame.skiaCanvas;
        canvas.drawPicture(
          _pictureRecorders[pictureRecorderIndex++].endRecording(),
        );
        frame.submit();
      }
    }
    for (final CkPictureRecorder recorder
        in _pictureRecordersCreatedDuringPreroll) {
      if (recorder.isRecording) {
        recorder.endRecording();
      }
    }
    _pictureRecordersCreatedDuringPreroll.clear();
    _pictureRecorders.clear();
    _seenFirstVisibleViewInPreroll = false;
    _seenFirstVisibleView = false;
    if (listEquals(_compositionOrder, _activeCompositionOrder)) {
      _compositionOrder.clear();
      _visibleViewCount = 0;
      return;
    }

    final Set<int> unusedViews = Set<int>.from(_activeCompositionOrder);
    _activeCompositionOrder.clear();

    List<int>? debugInvalidViewIds;

    if (diffResult != null) {
      // Dispose of the views that should be removed, except for the ones which
      // are going to be added back. Moving rather than removing and re-adding
      // the view helps it maintain state.
      disposeViews(diffResult.viewsToRemove
          .where((int view) => !diffResult.viewsToAdd.contains(view))
          .toSet());
      _activeCompositionOrder.addAll(_compositionOrder);
      unusedViews.removeAll(_compositionOrder);

      DomElement? elementToInsertBefore;
      if (diffResult.addToBeginning) {
        elementToInsertBefore =
            _viewClipChains[diffResult.viewToInsertBefore!]!.root;
      }

      for (final int viewId in diffResult.viewsToAdd) {
        if (assertionsEnabled) {
          if (!platformViewManager.knowsViewId(viewId)) {
            debugInvalidViewIds ??= <int>[];
            debugInvalidViewIds.add(viewId);
            continue;
          }
        }
        if (diffResult.addToBeginning) {
          final DomElement platformViewRoot = _viewClipChains[viewId]!.root;
          skiaSceneHost!.insertBefore(platformViewRoot, elementToInsertBefore);
          final Surface? overlay = _overlays[viewId];
          if (overlay != null) {
            skiaSceneHost!
                .insertBefore(overlay.htmlElement, elementToInsertBefore);
          }
        } else {
          final DomElement platformViewRoot = _viewClipChains[viewId]!.root;
          skiaSceneHost!.append(platformViewRoot);
          final Surface? overlay = _overlays[viewId];
          if (overlay != null) {
            skiaSceneHost!.append(overlay.htmlElement);
          }
        }
      }
      // It's possible that some platform views which were in the unchanged
      // section have newly assigned overlays. If so, add them to the DOM.
      for (int i = 0; i < _compositionOrder.length; i++) {
        final int view = _compositionOrder[i];
        if (_overlays[view] != null) {
          final DomElement overlayElement = _overlays[view]!.htmlElement;
          if (overlayElement.parent == null) {
            // This overlay wasn't added to the DOM.
            if (i == _compositionOrder.length - 1) {
              skiaSceneHost!.append(overlayElement);
            } else {
              final int nextView = _compositionOrder[i + 1];
              final DomElement nextElement = _viewClipChains[nextView]!.root;
              skiaSceneHost!.insertBefore(overlayElement, nextElement);
            }
          }
        }
      }
    } else {
      SurfaceFactory.instance.removeSurfacesFromDom();
      for (int i = 0; i < _compositionOrder.length; i++) {
        final int viewId = _compositionOrder[i];

        if (assertionsEnabled) {
          if (!platformViewManager.knowsViewId(viewId)) {
            debugInvalidViewIds ??= <int>[];
            debugInvalidViewIds.add(viewId);
            continue;
          }
        }

        final DomElement platformViewRoot = _viewClipChains[viewId]!.root;
        final Surface? overlay = _overlays[viewId];
        skiaSceneHost!.append(platformViewRoot);
        if (overlay != null) {
          skiaSceneHost!.append(overlay.htmlElement);
        }
        _activeCompositionOrder.add(viewId);
        unusedViews.remove(viewId);
      }
    }

    _compositionOrder.clear();
    _visibleViewCount = 0;

    disposeViews(unusedViews);

    if (assertionsEnabled) {
      if (debugInvalidViewIds != null && debugInvalidViewIds.isNotEmpty) {
        throw AssertionError(
          'Cannot render platform views: ${debugInvalidViewIds.join(', ')}. '
          'These views have not been created, or they have been deleted.',
        );
      }
    }
  }

  void disposeViews(Set<int> viewsToDispose) {
    for (final int viewId in viewsToDispose) {
      // Remove viewId from the _viewClipChains Map, and then from the DOM.
      final ViewClipChain? clipChain = _viewClipChains.remove(viewId);
      clipChain?.root.remove();
      // More cleanup
      _currentCompositionParams.remove(viewId);
      _viewsToRecomposite.remove(viewId);
      _cleanUpClipDefs(viewId);
      _svgClipDefs.remove(viewId);
    }
  }

  void _releaseOverlay(int viewId) {
    if (_overlays[viewId] != null) {
      final Surface overlay = _overlays[viewId]!;
      SurfaceFactory.instance.releaseSurface(overlay);
      _overlays.remove(viewId);
    }
  }

  // Assigns overlays to the embedded views in the scene.
  //
  // This method attempts to be efficient by taking advantage of the
  // [diffResult] and trying to re-use overlays which have already been
  // assigned.
  //
  // This method accounts for invisible platform views by grouping them
  // with the last visible platform view which precedes it. All invisible
  // platform views that come after a visible view share the same overlay
  // as the preceding visible view.
  //
  // This is called right before compositing the scene.
  //
  // [_compositionOrder] and [_activeComposition] order should contain the
  // composition order of the current and previous frame, respectively.
  //
  // TODO(hterkelsen): Test this more thoroughly.
  void _updateOverlays(ViewListDiffResult? diffResult) {
    if (diffResult != null &&
        diffResult.viewsToAdd.isEmpty &&
        diffResult.viewsToRemove.isEmpty) {
      // The composition order has not changed, continue using the assigned
      // overlays.
    }
    final List<List<int>> overlayGroups = getOverlayGroups(_compositionOrder);
    final List<int> viewsNeedingOverlays =
        overlayGroups.map((List<int> group) => group.last).toList();
    if (diffResult == null) {
      // Everything is going to be explicitly recomposited anyway. Release all
      // the surfaces and assign an overlay to the first N visible views where
      // N = [SurfaceFactory.instance.maximumOverlays] and assign the rest
      // to the backup surface.
      SurfaceFactory.instance.releaseSurfaces();
      _overlays.clear();
      viewsNeedingOverlays.forEach(_initializeOverlay);
    } else {
      // We want to preserve the overlays in the "unchanged" section of the
      // diff result as much as possible. Iterate over all the views needing
      // overlays and assign them an overlay if they don't have one already.
      final List<int> viewsWithOverlays = _overlays.keys.toList();
      for (final int view in viewsWithOverlays) {
        if (!viewsNeedingOverlays.contains(view)) {
          _releaseOverlay(view);
        }
      }
      for (final int view in viewsNeedingOverlays) {
        if (_overlays[view] == null) {
          _initializeOverlay(view);
        }
      }
    }
    assert(_overlays.length == viewsNeedingOverlays.length);
  }

  // Group the platform views into "overlay groups". These are sublists
  // of the composition order which can share the same overlay. Every overlay
  // group is a list containing a visible view followed by zero or more
  // invisible views.
  List<List<int>> getOverlayGroups(List<int> views) {
    // Visibility groups are typically a visible view followed by zero or more
    // invisible views. However, if the view list begins with one or more
    // invisible views, we can group them with the first visible view.
    final int maxGroups = SurfaceFactory.instance.maximumOverlays;
    if (maxGroups == 0) {
      return const <List<int>>[];
    }
    bool foundFirstVisibleView = false;
    final List<List<int>> result = <List<int>>[];
    List<int> currentGroup = <int>[];
    int i = 0;
    for (; i < views.length; i++) {
      // If we're on the last group, then break and just add all the rest of the
      // views to current group.
      if (result.length == maxGroups - 1) {
        break;
      }
      final int view = views[i];
      if (platformViewManager.isInvisible(view)) {
        currentGroup.add(view);
      } else {
        if (foundFirstVisibleView) {
          // We hit this case if this is the first visible view.
          result.add(currentGroup);
          currentGroup = <int>[view];
        } else {
          foundFirstVisibleView = true;
          currentGroup.add(view);
        }
      }
    }
    if (i < views.length) {
      currentGroup.addAll(views.sublist(i));
    }
    if (currentGroup.isNotEmpty) {
      result.add(currentGroup);
    }
    return result;
  }

  void _initializeOverlay(int viewId) {
    assert(!_overlays.containsKey(viewId));

    // Try reusing a cached overlay created for another platform view.
    final Surface overlay = SurfaceFactory.instance.getOverlay()!;
    overlay.createOrUpdateSurface(_frameSize);
    _overlays[viewId] = overlay;
  }

  /// Deletes SVG clip paths, useful for tests.
  void debugCleanupSvgClipPaths() {
    final DomElement? parent = _svgPathDefs?.children.single;
    if (parent != null) {
      for (DomNode? child = parent.lastChild;
          child != null;
          child = parent.lastChild) {
        parent.removeChild(child);
      }
    }
    _svgClipDefs.clear();
  }

  static void removeElement(DomElement element) {
    element.remove();
  }

  /// Clears the state of this view embedder. Used in tests.
  void debugClear() {
    final Set<int> allViews = platformViewManager.debugClear();
    disposeViews(allViews);
    _pictureRecorders.clear();
    _currentCompositionParams.clear();
    debugCleanupSvgClipPaths();
    _currentCompositionParams.clear();
    _viewClipChains.clear();
    _overlays.clear();
    _viewsToRecomposite.clear();
    _activeCompositionOrder.clear();
    _compositionOrder.clear();
    _visibleViewCount = 0;
  }
}

/// Represents a Clip Chain (for a view).
///
/// Objects of this class contain:
/// * The root view in the stack of mutator elements for the view id.
/// * The slot view in the stack (the actual contents of the platform view).
/// * The number of clipping elements used last time the view was composited.
class ViewClipChain {
  DomElement _root;
  DomElement _slot;
  int _clipCount = -1;

  ViewClipChain({required DomElement view})
      : _root = view,
        _slot = view;

  DomElement get root => _root;
  DomElement get slot => _slot;
  int get clipCount => _clipCount;

  void updateClipChain({required DomElement root, required int clipCount}) {
    _root = root;
    _clipCount = clipCount;
  }
}

/// The parameters passed to the view embedder.
class EmbeddedViewParams {
  EmbeddedViewParams(this.offset, this.size, MutatorsStack mutators)
      : mutators = MutatorsStack._copy(mutators);

  final ui.Offset offset;
  final ui.Size size;
  final MutatorsStack mutators;

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    return other is EmbeddedViewParams &&
        other.offset == offset &&
        other.size == size &&
        other.mutators == mutators;
  }

  @override
  int get hashCode => Object.hash(offset, size, mutators);
}

enum MutatorType {
  clipRect,
  clipRRect,
  clipPath,
  transform,
  opacity,
}

/// Stores mutation information like clipping or transform.
class Mutator {
  const Mutator._(
    this.type,
    this.rect,
    this.rrect,
    this.path,
    this.matrix,
    this.alpha,
  );

  final MutatorType type;
  final ui.Rect? rect;
  final ui.RRect? rrect;
  final ui.Path? path;
  final Matrix4? matrix;
  final int? alpha;

  const Mutator.clipRect(ui.Rect rect)
      : this._(MutatorType.clipRect, rect, null, null, null, null);
  const Mutator.clipRRect(ui.RRect rrect)
      : this._(MutatorType.clipRRect, null, rrect, null, null, null);
  const Mutator.clipPath(ui.Path path)
      : this._(MutatorType.clipPath, null, null, path, null, null);
  const Mutator.transform(Matrix4 matrix)
      : this._(MutatorType.transform, null, null, null, matrix, null);
  const Mutator.opacity(int alpha)
      : this._(MutatorType.opacity, null, null, null, null, alpha);

  bool get isClipType =>
      type == MutatorType.clipRect ||
      type == MutatorType.clipRRect ||
      type == MutatorType.clipPath;

  double get alphaFloat => alpha! / 255.0;

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    if (other is! Mutator) {
      return false;
    }

    final Mutator typedOther = other;
    if (type != typedOther.type) {
      return false;
    }

    switch (type) {
      case MutatorType.clipRect:
        return rect == typedOther.rect;
      case MutatorType.clipRRect:
        return rrect == typedOther.rrect;
      case MutatorType.clipPath:
        return path == typedOther.path;
      case MutatorType.transform:
        return matrix == typedOther.matrix;
      case MutatorType.opacity:
        return alpha == typedOther.alpha;
      default:
        return false;
    }
  }

  @override
  int get hashCode => Object.hash(type, rect, rrect, path, matrix, alpha);
}

/// A stack of mutators that can be applied to an embedded view.
class MutatorsStack extends Iterable<Mutator> {
  MutatorsStack() : _mutators = <Mutator>[];

  MutatorsStack._copy(MutatorsStack original)
      : _mutators = List<Mutator>.from(original._mutators);

  final List<Mutator> _mutators;

  void pushClipRect(ui.Rect rect) {
    _mutators.add(Mutator.clipRect(rect));
  }

  void pushClipRRect(ui.RRect rrect) {
    _mutators.add(Mutator.clipRRect(rrect));
  }

  void pushClipPath(ui.Path path) {
    _mutators.add(Mutator.clipPath(path));
  }

  void pushTransform(Matrix4 matrix) {
    _mutators.add(Mutator.transform(matrix));
  }

  void pushOpacity(int alpha) {
    _mutators.add(Mutator.opacity(alpha));
  }

  void pop() {
    _mutators.removeLast();
  }

  @override
  bool operator ==(Object other) {
    if (identical(other, this)) {
      return true;
    }
    return other is MutatorsStack &&
        listEquals<Mutator>(other._mutators, _mutators);
  }

  @override
  int get hashCode => Object.hashAll(_mutators);

  @override
  Iterator<Mutator> get iterator => _mutators.reversed.iterator;
}

/// The results of diffing the current composition order with the active
/// composition order.
class ViewListDiffResult {
  /// Views which should be removed from the scene.
  final List<int> viewsToRemove;

  /// Views to add to the scene.
  final List<int> viewsToAdd;

  /// If `true`, [viewsToAdd] should be added at the beginning of the scene.
  /// Otherwise, they should be added at the end of the scene.
  final bool addToBeginning;

  /// If [addToBeginning] is `true`, then this is the id of the platform view
  /// to insert [viewsToAdd] before.
  ///
  /// `null` if [addToBeginning] is `false`.
  final int? viewToInsertBefore;

  const ViewListDiffResult(
      this.viewsToRemove, this.viewsToAdd, this.addToBeginning,
      {this.viewToInsertBefore});
}

/// Diff the composition order with the active composition order. It is
/// common for the composition order and active composition order to differ
/// only slightly.
///
/// Consider a scrolling list of platform views; from frame
/// to frame the composition order will change in one of two ways, depending
/// on which direction the list is scrolling. One or more views will be added
/// to the beginning of the list, and one or more views will be removed from
/// the end of the list, with the order of the unchanged middle views
/// remaining the same.
// TODO(hterkelsen): Refactor to use [longestIncreasingSubsequence] and logic
// similar to `Surface._insertChildDomNodes` to efficiently handle more cases,
// https://github.com/flutter/flutter/issues/89611.
ViewListDiffResult? diffViewList(List<int> active, List<int> next) {
  if (active.isEmpty || next.isEmpty) {
    return null;
  }

  // This is tried if the first element of the next list is contained in the
  // active list at `index`. If the active and next lists are in the expected
  // form, then we should be able to iterate from `index` to the end of the
  // active list where every element matches in the next list.
  ViewListDiffResult? lookForwards(int index) {
    for (int i = 0; i + index < active.length; i++) {
      if (active[i + index] != next[i]) {
        // An element in the next list didn't match. This isn't in the expected
        // form we can optimize.
        return null;
      }
      if (i == next.length - 1) {
        // The entire next list was contained in the active list.
        if (index == 0) {
          // If the first index of the next list is also the first index in the
          // active list, then the next list is the same as the active list with
          // views removed from the end.
          return ViewListDiffResult(
              active.sublist(i + 1), const <int>[], false);
        } else if (i + index == active.length - 1) {
          // If this is also the end of the active list, then the next list is
          // the same as the active list with some views removed from the
          // beginning.
          return ViewListDiffResult(
              active.sublist(0, index), const <int>[], false);
        } else {
          return null;
        }
      }
    }
    // We reached the end of the active list but have not reached the end of the
    // next list. The lists are in the expected form. We should remove the
    // elements from `0` to `index` in the active list from the DOM and add the
    // elements from `active.length - index` (the entire active list minus the
    // number of new elements at the beginning) to the end of the next list to
    // the DOM at the end of the list of platform views.
    final List<int> viewsToRemove = active.sublist(0, index);
    final List<int> viewsToAdd = next.sublist(active.length - index);

    return ViewListDiffResult(
      viewsToRemove,
      viewsToAdd,
      false,
    );
  }

  // This is tried if the last element of the next list is contained in the
  // active list at `index`. If the lists are in the expected form, we should be
  // able to iterate backwards from index to the beginning of the active list
  // and have every element match the corresponding element from the next list.
  ViewListDiffResult? lookBackwards(int index) {
    for (int i = 0; index - i >= 0; i++) {
      if (active[index - i] != next[next.length - 1 - i]) {
        // An element from the next list didn't match the coresponding element
        // from the active list. These lists aren't in the expected form.
        return null;
      }
      if (i == next.length - 1) {
        // The entire next list was contained in the active list.
        if (index == active.length - 1) {
          // If the last element of the next list is also the last element of
          // the active list, then the next list is just the active list with
          // some elements removed from the beginning.
          return ViewListDiffResult(
              active.sublist(0, active.length - i - 1), const <int>[], false);
        } else if (index == i) {
          // If we also reached the beginning of the active list, then the next
          // list is the same as the active list with some views removed from
          // the end.
          return ViewListDiffResult(
              active.sublist(index + 1), const <int>[], false);
        } else {
          return null;
        }
      }
    }

    // We reached the beginning of the active list but have not exhausted the
    // entire next list. The lists are in the expected form. We should remove
    // the elements from the end of the active list which come after the element
    // which matches the last index of the next list (everything after `index`).
    // We should add the elements from the next list which we didn't reach while
    // iterating above (the first `next.length - index` views).
    final List<int> viewsToRemove = active.sublist(index + 1);
    final List<int> viewsToAdd = next.sublist(0, next.length - 1 - index);

    return ViewListDiffResult(
      viewsToRemove,
      viewsToAdd,
      true,
      viewToInsertBefore: active.first,
    );
  }

  // If the [active] and [next] lists are in the expected form described above,
  // then either the first or last element of [next] will be in [active].
  final int firstIndex = active.indexOf(next.first);
  final int lastIndex = active.lastIndexOf(next.last);
  if (firstIndex != -1 && lastIndex != -1) {
    // Both the first element and the last element of the next list are in the
    // active list. Search in the direction that will result in the least
    // amount of deletions.
    if (firstIndex <= (active.length - lastIndex)) {
      return lookForwards(firstIndex);
    } else {
      return lookBackwards(lastIndex);
    }
  } else if (firstIndex != -1) {
    return lookForwards(firstIndex);
  } else if (lastIndex != -1) {
    return lookBackwards(lastIndex);
  } else {
    return null;
  }
}
