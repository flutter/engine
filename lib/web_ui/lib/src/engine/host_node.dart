// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// The interface required to host a flutter app in the DOM, and its tests.
///
/// Consider this as the intersection in functionality between [html.ShadowRoot]
/// (preferred Flutter rendering method) and [html.Document] (fallback).
///
/// Not to be confused with [html.DocumentOrShadowRoot].
abstract class HostNode {
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

/// A partial [HostNode] implementation, backed by a [html.Node].
///
/// The implementation is partial (abstract) because Nodes alone can't implement
/// the complete functionality.
///
/// The late property `_node` must be set in the extender's constructor.
abstract class NodeHostNode implements HostNode {
  late html.Node _node;

  @override
  html.Node append(html.Node node) {
    return _node.append(node);
  }

  @override
  bool contains(html.Node? other) {
    return _node.contains(other);
  }

  @override
  html.Node get node => _node;

  @override
  List<html.Node> get nodes => _node.nodes;
}

/// A [HostNode] implementation, backed by a [html.ShadowRoot].
///
/// This is the preferred flutter implementation, but it might not be supported
/// by all browsers yet.
///
/// The constructor might throw when calling `attachShadow`, if ShadowDOM is not
/// supported in the current environment. In this case, a fallback [ElementHostNode]
/// should be created instead.
class ShadowDomHostNode extends NodeHostNode {
  late html.ShadowRoot _shadow;

  /// Build a HostNode by attaching a [html.ShadowRoot] to the `root` element.
  ShadowDomHostNode(html.Element root) {
    _shadow = _node = root.attachShadow(<String, String>{
      'mode': 'open',
      'delegatesFocus': 'true',
    });
  }

  @override
  html.Element? get activeElement => _shadow.activeElement;

  @override
  html.Element? querySelector(String selectors) {
    return _shadow.querySelector(selectors);
  }

  @override
  List<html.Node> querySelectorAll(String selectors) {
    return _shadow.querySelectorAll(selectors);
  }
}

/// A [HostNode] implementation, backed by a [html.Element].
///
/// This is a fallback implementation, in case [ShadowDomHostNode] fails when
/// being constructed.
class ElementHostNode extends NodeHostNode {
  late html.Element _element;

  /// Build a HostNode by attaching a child [html.Element] to the `root` element.
  ElementHostNode(html.Element root) {
    _element = _node = html.document.createElement('flt-shadow-dom-fallback');
    root.append(_element);
  }

  @override
  html.Element? get activeElement => _element.ownerDocument?.activeElement;

  @override
  html.Element? querySelector(String selectors) {
    return _element.querySelector(selectors);
  }

  @override
  List<html.Node> querySelectorAll(String selectors) {
    return _element.querySelectorAll(selectors);
  }
}
