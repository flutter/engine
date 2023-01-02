// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:ui/ui.dart' as ui;

import '../../engine/color_filter.dart';
import '../dom.dart';
import '../embedder.dart';
import 'shaders/shader.dart';
import 'surface.dart';

/// A surface that applies an [ColorFilter] to its children.
class PersistedColorFilter extends PersistedContainerSurface
    implements ui.ColorFilterEngineLayer {
  PersistedColorFilter(PersistedColorFilter? super.oldLayer, this.filter);

  @override
  DomElement? get childContainer => _childContainer;

  /// The dedicated child container element that's separate from the
  /// [rootElement] is used to compensate for the coordinate system shift
  /// introduced by the [rootElement] translation.
  DomElement? _childContainer;

  /// Color filter to apply to this surface.
  final ui.ColorFilter filter;
  DomElement? _filterElement;
  bool containerVisible = true;

  @override
  void adoptElements(PersistedColorFilter oldSurface) {
    super.adoptElements(oldSurface);
    _childContainer = oldSurface._childContainer;
    _filterElement = oldSurface._filterElement;
    oldSurface._childContainer = null;
  }

  @override
  void preroll(PrerollSurfaceContext prerollContext) {
    ++prerollContext.activeColorFilterCount;
    super.preroll(prerollContext);
    --prerollContext.activeColorFilterCount;
  }

  @override
  void discard() {
    super.discard();
    flutterViewEmbedder.removeResource(_filterElement);
    _filterElement = null;
    // Do not detach the child container from the root. It is permanently
    // attached. The elements are reused together and are detached from the DOM
    // together.
    _childContainer = null;
  }

  @override
  DomElement createElement() {
    final DomElement element = defaultCreateElement('flt-color-filter');
    final DomElement container = createDomElement('flt-filter-interior');
    container.style.position = 'absolute';
    _childContainer = container;
    element.append(_childContainer!);
    return element;
  }

  @override
  void apply() {
    flutterViewEmbedder.removeResource(_filterElement);
    _filterElement = null;
    final EngineHtmlColorFilter? engineValue = createHtmlColorFilter(filter as EngineColorFilter);
    if (engineValue == null) {
      rootElement!.style.backgroundColor = '';
      childContainer?.style.visibility = 'visible';
      return;
    }

    if (engineValue is ModeHtmlColorFilter) {
      _applyBlendModeFilter(engineValue);
    } else if (engineValue is MatrixHtmlColorFilter) {
      _applyMatrixColorFilter(engineValue);
    } else {
      childContainer?.style.visibility = 'visible';
    }
  }

  void _applyBlendModeFilter(ModeHtmlColorFilter colorFilter) {
    _filterElement = colorFilter.makeSvgFilter(childContainer);

    /// Some blendModes do not make an svgFilter. See [EngineHtmlColorFilter.makeSvgFilter()]
    if (_filterElement == null) {
      return;
    }
    childContainer!.style.filter = colorFilter.filterAttribute;
  }

  void _applyMatrixColorFilter(MatrixHtmlColorFilter colorFilter) {
    _filterElement = colorFilter.makeSvgFilter(childContainer);
    childContainer!.style.filter = colorFilter.filterAttribute;
  }

  @override
  void update(PersistedColorFilter oldSurface) {
    super.update(oldSurface);

    if (oldSurface.filter != filter) {
      apply();
    }
  }
}
