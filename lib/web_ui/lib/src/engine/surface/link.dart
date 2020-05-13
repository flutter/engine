// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

/// A surface that translates its children using CSS transform and translate.
class PersistedLink extends PersistedContainerSurface
    implements ui.LinkEngineLayer {
  PersistedLink(
    PersistedLink oldLayer, {
    this.destination,
    this.label,
    this.target,
    this.rect,
  }) : super(oldLayer);

  ///
  final String destination;

  ///
  final String label;

  ///
  final int target;

  ///
  final ui.Rect rect;

  ///
  @override
  html.Element get childContainer => _childContainer;
  html.Element _childContainer;

  @override
  void adoptElements(PersistedLink oldSurface) {
    super.adoptElements(oldSurface);
    _childContainer = oldSurface._childContainer;
    oldSurface._childContainer = null;
  }

  @override
  html.Element createElement() {
    final html.Element element = defaultCreateElement('a');
    element.style
      ..transformOrigin = '0 0 0'
      ..pointerEvents = 'auto';
    if (assertionsEnabled) {
      element..setAttribute('data-debug-tag', 'flt-link');
    }

    _childContainer = html.Element.tag('flt-link-interior');
    _childContainer.style.position = 'relative';
    element.append(_childContainer);
    if (_debugExplainSurfaceStats) {
      // This creates an additional interior element. Count it too.
      _surfaceStatsFor(this).allocatedDomNodeCount++;
    }

    return element;
  }

  @override
  void discard() {
    super.discard();
    _childContainer = null;
  }

  @override
  void apply() {
    rootElement
      ..setAttribute('href', destination)
      ..setAttribute('alt', label)
      ..setAttribute('target', _getHtmlTarget(target));
    _setRect(rect);
  }

  @override
  void update(PersistedLink oldSurface) {
    super.update(oldSurface);
    if (destination != oldSurface.destination) {
      rootElement.setAttribute('href', destination);
    }
    if (label != oldSurface.label) {
      rootElement.setAttribute('alt', label);
    }
    if (target != oldSurface.target) {
      rootElement.setAttribute('target', _getHtmlTarget(target));
    }
    if (rect != oldSurface.rect) {
      _setRect(rect);
    }
  }

  void _setRect(ui.Rect rect) {
    // Because the <a> element is absolutely positioned, it doesn't grow to fit
    // its children. We have explicitly set width and height in order for it to
    // occupy space on the screen.
    rootElement.style
      ..transform = 'translate(${rect.left}px, ${rect.top}px)'
      ..width = '${rect.width}px'
      ..height = '${rect.height}px';

    // The same offset given to PersistedLink is also given to its children. In
    // order to avoid double-offsetting the children, we wrap them in a
    // container with an equal, negative offset.
    _childContainer.style
      ..left = '-${rect.left}px'
      ..top = '-${rect.top}px';
  }
}

String _getHtmlTarget(int target) {
  switch (target) {
    case 0: // LinkTarget.defaultTarget
    case 1: // LinkTarget.self
      return '_self';
    case 2: // LinkTarget.blank
      return '_blank';
    default:
      throw Exception('Unknown LinkTarget value $target.');
  }
}
