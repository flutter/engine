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

/// Keeps pointers to factories to create platform views, the created platform views,
/// and the slot tags for these platform views.
class PlatformViewManager {
  // The factory functions, indexed by the viewType
  final Map<String, Function> _factories = {};

  // The references to <slot> tags, indexed by their framework-given ID.
  final Map<int, html.SlotElement> _slots = {};

  // The references to content tags, indexed by their framework-given ID.
  final Map<int, html.Element> _content = {};

  String _getSlotName(String viewType, int viewId) {
    return 'flt-pv-${viewType}-$viewId';
  }

  bool knowsViewType(String viewType) {
    return _factories.containsKey(viewType);
  }

  bool knowsViewId(int viewId) {
    return _content.containsKey(viewId);
  }

  bool registerFactory(String platformViewId, Function factoryFunction) {
    assert(factoryFunction is PlatformViewFactory ||
        factoryFunction is ParameterizedPlatformViewFactory);

    if (_factories.containsKey(platformViewId)) {
      return false;
    }
    _factories[platformViewId] = factoryFunction;
    return true;
  }

  html.SlotElement getSlot(int viewId) {
    assert(_slots.containsKey(viewId));
    return _slots[viewId]!;
  }

  html.SlotElement renderSlot(String viewType, int viewId) {
    assert(!knowsViewType(viewType),
        'Attempted to render slot of unregistered viewType: $viewType');

    final String slotName = _getSlotName(viewType, viewId);

    return _slots.putIfAbsent(viewId, () {
      final html.Element slot = html.document.createElement('slot')
        ..setAttribute('name', slotName)
        ..style.pointerEvents = 'auto';
      return slot as html.SlotElement;
    });
  }

  html.Element getContent(int viewId) {
    assert(_content.containsKey(viewId));
    return _content[viewId]!;
  }

  html.Element renderContent(
    String viewType,
    int viewId,
    Map<dynamic, dynamic>? params,
  ) {
    assert(!knowsViewType(viewType),
        'Attempted to render contents of unregistered viewType: $viewType');

    final String slotName = _getSlotName(viewType, viewId);

    return _content.putIfAbsent(viewId, () {
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

      // We could move this to a CSS stylesheet
      content.style.height = 'inherit';

      return wrapper..append(content);
    });
  }

  void clearPlatformView(int viewId) {
    // Remove from our cache, and then from the DOM...
    _slots.remove(viewId)?.remove();
    _content.remove(viewId)?.remove();
  }
}
