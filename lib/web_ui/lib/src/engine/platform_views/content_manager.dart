// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

// import 'dart:html' as html;

/// A function which takes a unique `id` and some `params` and creates an HTML element.
///
/// This is made available to end-users through dart:ui in web.
typedef ParameterizedPlatformViewFactory = html.Element Function(
  int viewId, {
  Map<dynamic, dynamic>? params,
});
/// A function which takes a unique `id` and creates an HTML element.
///
/// This is made available to end-users through dart:ui in web.
typedef PlatformViewFactory = html.Element Function(int viewId);

/// This class handles the lifecycle of Platform Views in the DOM of a Flutter Web App.
///
/// There are three important parts of Platform Views that this class manages:
///
/// * `factories`: The functions used to render the contents of any given Platform
/// View by its `viewType`.
/// * `contents`: The result [html.Element] of calling a `factory` function.
/// * `slots`: A special HTML tag that will be used by the framework to "reveal"
/// the `contents` of a Platform View.
///
/// This class keeps a registry of `factories`, `slots` and `contents` so the
/// framework can CRUD Platform Views as needed, regardless of the rendering backend.
class PlatformViewManager {
  // The factory functions, indexed by the viewType
  final Map<String, Function> _factories = {};

  // The references to <slot> tags, indexed by their framework-given ID.
  final Map<int, html.Element> _slots = {};

  // The references to content tags, indexed by their framework-given ID.
  final Map<int, html.Element> _contents = {};

  String _getSlotName(String viewType, int viewId) {
    return 'flt-pv-${viewType}-$viewId';
  }

  /// Returns `true` if the passed in `viewType` has been registered before.
  ///
  /// See [registerViewFactory] to understand how factories are registered.
  bool knowsViewType(String viewType) {
    return _factories.containsKey(viewType);
  }

  /// Returns `true` if the passed in `viewId` has been rendered (and not disposed) before.
  ///
  /// See [renderContent] and [renderSlot] to understand how platform views are
  /// rendered.
  bool knowsViewId(int viewId) {
    return _contents.containsKey(viewId);
  }

  /// Registers a `factoryFunction` that knows how to render a Platform View of `viewType`.
  ///
  /// `viewType` is selected by the programmer, but it can't be overridden once
  /// it's been set.
  ///
  /// `factoryFunction` needs to be a [PlatformViewFactory].
  bool registerFactory(String viewType, Function factoryFunction) {
    assert(factoryFunction is PlatformViewFactory ||
        factoryFunction is ParameterizedPlatformViewFactory);

    if (_factories.containsKey(viewType)) {
      return false;
    }
    _factories[viewType] = factoryFunction;
    return true;
  }

  /// Returns the `slot` associated to a `viewId`, if it's been rendered (and not disposed of).
  ///
  /// See [renderSlot] to understand how `slots` are rendered.
  html.Element getSlot(int viewId) {
    assert(_slots.containsKey(viewId));
    return _slots[viewId]!;
  }

  /// Creates the HTML markup for the `slot` of a Platform View.
  ///
  /// The result of this call is cached in the `_slots` Map, so it can be accessed
  /// later for CRUD operations.
  ///
  /// The resulting DOM for a `slot` looks like this:
  ///
  /// ```html
  /// <flt-platform-view-slot style="...">
  ///   <slot name="..." />
  /// </flt-platform-view-slot>
  /// ```
  ///
  /// The inner `SLOT` tag is standard HTML to reveal an element that is rendered
  /// elsewhere in the DOM. Its `name` attribute must match the value of the `slot`
  /// attribute of the contents being revealed (see [renderContent].)
  ///
  /// The outer `flt-platform-view-slot` tag is a simple wrapper that the framework
  /// can position/style as needed.
  ///
  /// (When the framework accesses a `slot`, it's really accessing its wrapper
  /// `flt-platform-view-slot` tag)
  html.Element renderSlot(String viewType, int viewId) {
    assert(knowsViewType(viewType),
        'Attempted to render slot of unregistered viewType: $viewType');

    final String slotName = _getSlotName(viewType, viewId);

    return _slots.putIfAbsent(viewId, () {
      final html.Element wrapper = html.document
          .createElement('flt-platform-view-slot')
          ..style.pointerEvents = 'auto';

      final html.Element slot = html.document.createElement('slot')
        ..setAttribute('name', slotName);

      return wrapper..append(slot);
    });
  }

  /// Creates the HTML markup for the `contents` of a Platform View.
  ///
  /// The result of this call is cached in the `_contents` Map. This is only
  /// cached so it can be disposed of later by [clearPlatformView]. _Note that
  /// there's no `getContents` function in this class._
  ///
  /// The resulting DOM for the `contents` of a Platform View looks like this:
  ///
  /// ```html
  /// <flt-platform-view slot="...">
  ///   <arbitrary-html-elements />
  /// </flt-platform-view-slot>
  /// ```
  ///
  /// The `arbitrary-html-elements` are the result of the call to the user-supplied
  /// `factory` function for this Platform View (see [registerFactory]).
  ///
  /// The outer `flt-platform-view` tag is a simple wrapper that we add to have
  /// a place where to attach the `slot` property, that will tell the browser
  /// what `slot` tag will reveal this `contents`, **without modifying the returned
  /// html from the `factory` function**.
  html.Element renderContent(
    String viewType,
    int viewId,
    Map<dynamic, dynamic>? params,
  ) {
    assert(knowsViewType(viewType),
        'Attempted to render contents of unregistered viewType: $viewType');

    final String slotName = _getSlotName(viewType, viewId);

    return _contents.putIfAbsent(viewId, () {
      final html.Element wrapper = html.document
          .createElement('flt-platform-view')
            ..setAttribute('slot', slotName);

      final Function factoryFunction = _factories[viewType]!;
      late html.Element content;

      if (factoryFunction is ParameterizedPlatformViewFactory) {
        content = factoryFunction(viewId, params: params);
      } else {
        content = factoryFunction(viewId);
      }

      // Scrutinize closely any other modifications to `content`.
      // We shouldn't modify users' returned `content` if at all possible.
      // Note there's also no getContent(viewId) function anymore, to prevent
      // from later modifications too.
      if (content.style.height.isEmpty) {
        printWarning('Height of Platform View type: [$viewType] may not be set. Defaulting to `height: 100%`.\nSet `style.height` to any appropriate value to stop this message.');
        content.style.height = '100%';
      }

      return wrapper..append(content);
    });
  }

  /// Removes a PlatformView by its `viewId` from the manager, and from the DOM.
  ///
  /// Once a view has been cleared, calls to [getSlot] or [knowsViewId] will fail,
  /// as if it had never been rendered before.
  void clearPlatformView(int viewId) {
    // Remove from our cache, and then from the DOM...
    _slots.remove(viewId)?.remove();
    _contents.remove(viewId)?.remove();
  }
}
