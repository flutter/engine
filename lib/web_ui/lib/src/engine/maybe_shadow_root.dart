// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// This interface defines the common functionality of [html.ShadowRoot] and [html.Document].
///
/// Do not confuse this with [html.DocumentOrShadowRoot].
abstract class ShadowRootOrDocument {
  /// Retrieves the [html.Element] that currently has focus.
  ///
  /// See:
  /// * [Document.activeElement](https://developer.mozilla.org/en-US/docs/Web/API/Document/activeElement)
  html.Element? get activeElement;

  /// Adds a node to the end of the child [nodes] list of this node.
  ///
  /// If the node already exists in this document, it will be removed from its
  /// current parent node, then added to this node.
  ///
  /// This method is more efficient than `nodes.add`, and is the preferred
  /// way of appending a child node.
  ///
  /// See:
  /// * [Node.appendChild](https://developer.mozilla.org/en-US/docs/Web/API/Node/appendChild)
  html.Node append(html.Node node);

  /// Returns true if this node contains the specified node.
  /// See:
  /// * [Node.contains](https://developer.mozilla.org/en-US/docs/Web/API/Node.contains)
  bool contains(html.Node? other);

  /// Returns the currently wrapped [html.Node].
  html.Node get node;

  /// A modifiable list of this node's children.
  List<html.Node> get nodes;

  /// Finds the first descendant element of this document that matches the
  /// specified group of selectors.
  ///
  /// [selectors] should be a string using CSS selector syntax.
  ///
  /// ```dart
  /// var element1 = document.querySelector('.className');
  /// var element2 = document.querySelector('#id');
  /// ```
  ///
  /// For details about CSS selector syntax, see the
  /// [CSS selector specification](http://www.w3.org/TR/css3-selectors/).
  ///
  /// See:
  /// * [Document.querySelector](https://developer.mozilla.org/en-US/docs/Web/API/Document/querySelector)
  html.Element? querySelector(String selectors);

  /// Finds all descendant elements of this document that match the specified
  /// group of selectors.
  ///
  /// [selectors] should be a string using CSS selector syntax.
  ///
  /// ```dart
  /// var items = document.querySelectorAll('.itemClassName');
  /// ```
  ///
  /// For details about CSS selector syntax, see the
  /// [CSS selector specification](http://www.w3.org/TR/css3-selectors/).
  ///
  /// See:
  /// * [Document.querySelectorAll](https://developer.mozilla.org/en-US/docs/Web/API/Document/querySelectorAll)
  List<html.Node> querySelectorAll(String selectors);
}

/// A DOM Wrapper class that attempts to create a [html.ShadowRoot] on a target
/// [html.Element], and falls back to a child [html.Element] if unable to do so.
class MaybeShadowRoot implements ShadowRootOrDocument {

  late html.ShadowRoot? _shadow;
  late html.Element _fallback;

  /// Constructs a MaybeShadowDom for a given `root` [html.Element].
  MaybeShadowRoot(html.Element root, {
    @visibleForTesting bool debugForceFallback = false,
  }) {
    if (debugForceFallback) {
      _initFallbackDom(root);
    } else {
      try {
        _initShadowDom(root);
      } catch(e) {
        _initFallbackDom(root);
      }
    }
  }

  // Initializes _element as a direct child of a given `parent`.
  void _initFallbackDom(html.Element root) {
    _shadow = null;
    _fallback = html.document.createElement('flt-shadow-dom-fallback');
    root.append(_fallback);
  }

  // Initializes _element as the shadowRoot of `parent`.
  void _initShadowDom(html.Element root) {
    _shadow = root.attachShadow(<String, String>{
      'mode': 'open',
      'delegatesFocus': 'true',
    });
  }

  /// Returns `true` if this MaybeShadowRoot is actually a ShadowRoot.
  bool get isShadowDom => _shadow != null;

  @override
  html.Element? get activeElement {
    return (isShadowDom ? _shadow!.activeElement : _fallback.ownerDocument?.activeElement);
  }

  @override
  html.Node append(html.Node node) {
    return (_shadow ?? _fallback).append(node);
  }

  @override
  bool contains(html.Node? other) {
    return (_shadow ?? _fallback).contains(other);
  }

  @override
  html.Node get node => (_shadow ?? _fallback);

  @override
  List<html.Node> get nodes => (_shadow ?? _fallback).nodes;

  @override
  html.Element? querySelector(String selectors) {
    return (isShadowDom ? _shadow!.querySelector(selectors) : _fallback.querySelector(selectors));
  }

  @override
  List<html.Node> querySelectorAll(String selectors) {
    return (isShadowDom ? _shadow!.querySelectorAll(selectors) : _fallback.querySelectorAll(selectors));
  }
}
