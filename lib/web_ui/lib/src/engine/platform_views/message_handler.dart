// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

// import 'dart:html' as html;

// import 'dart:typed_data';

// import '../../engine.dart' show MethodCodec, MethodCall, StandardMethodCodec;
// import '../../../ui.dart' as ui show PlatformMessageResponseCallback;
// import './content_manager.dart';

/// A function that handles a newly created [html.SlotElement] for a platform
/// view with a unique [int] id.
typedef PlatformViewSlotHandler = void Function(int, html.SlotElement);

/// A function that handle a newly created [html.Element] with the contents of a
/// platform view with a unique [int] id.
typedef PlatformViewContentHandler = void Function(int, html.Element);

/// A function that handles part of the `dispose` lifecycle of a platform view
/// with a unique [int] id.
typedef PlatformViewDisposeHandler = void Function(int);

/// This class handles incoming framework messages to create/dispose Platform Views.
///
/// It uses a [PlatformViewManager] to manage the creation of the DOM of
/// the PlatformViews. This `contentManager` is shared across the engine, to perform
/// all operations related to platform views (registration, rendering, etc...)
///
/// When a platform view is created, the injection into the App DOM is delegated
/// to a [PlatformViewSlotHandler] and a [ConventViewHandler] function, which will
/// decide where in the global DOM to inject the different components of the view.
///
/// When a platform view is disposed of, it is removed from the cache (and DOM)
/// directly by the `contentManager`. However, we give the framework a final chance
/// to do cleanup by calling a [PlatformViewDisposeHandler] with the `viewId` that
/// is being disposed of.
class PlatformViewMessageHandler {
  final MethodCodec _codec = StandardMethodCodec();

  final PlatformViewManager _contentManager;
  final PlatformViewSlotHandler? _slotHandler;
  final PlatformViewContentHandler? _contentHandler;
  final PlatformViewDisposeHandler? _disposeHandler;

  PlatformViewMessageHandler({
    required PlatformViewManager contentManager,
    PlatformViewSlotHandler? slotHandler,
    PlatformViewContentHandler? contentHandler,
    PlatformViewDisposeHandler? disposeHandler,
  }) : this._contentManager = contentManager,
       this._slotHandler = slotHandler,
       this._contentHandler = contentHandler,
       this._disposeHandler = disposeHandler;

  void _createPlatformView(
    MethodCall methodCall,
    ui.PlatformMessageResponseCallback callback,
  ) {
    final Map<dynamic, dynamic> args = methodCall.arguments;
    final int viewId = args['id'];
    final String viewType = args['viewType'];

    if (!_contentManager.knowsViewType(viewType)) {
      callback(_codec.encodeErrorEnvelope(
        code: 'Unregistered factory',
        message: "No factory registered for viewtype '$viewType'",
      ));
      return;
    }

    final html.Element content = _contentManager.renderContent(
      viewType,
      viewId,
      args,
    );
    final html.SlotElement slot = _contentManager.renderSlot(viewType, viewId);

    try {
      // For now, we don't need anything fancier. If needed, this can be converted
      // to a PlatformViewStrategy class for each web-renderer backend.
      // The Strategy can also be expanded to take care of disposal, too.
      if (_contentHandler != null) {
        _contentHandler!(viewId, content);
      }
      if (_slotHandler != null) {
        _slotHandler!(viewId, slot);
      }

      callback(_codec.encodeSuccessEnvelope(null));
    } catch (e) {
      callback(_codec.encodeErrorEnvelope(
        code: 'Failure rendering platform view',
        message: e.toString(),
      ));
    }
  }

  void _disposePlatformView(
    MethodCall methodCall,
    ui.PlatformMessageResponseCallback callback,
  ) {
    final int viewId = methodCall.arguments;

    // The contentManager removes the slot and the contents from its internal
    // cache, and the DOM.
    _contentManager.clearPlatformView(viewId);

    // However, the canvaskit renderer needs to perform some extra cleanup, so
    // we call it now.
    if (_disposeHandler != null) {
      _disposeHandler!(viewId);
    }

    callback(_codec.encodeSuccessEnvelope(null));
  }

  void handlePlatformViewCall(
    ByteData? data,
    ui.PlatformMessageResponseCallback callback,
  ) {
    final MethodCall decoded = _codec.decodeMethodCall(data);
    switch (decoded.method) {
      case 'create':
        _createPlatformView(decoded, callback);
        return;
      case 'dispose':
        _disposePlatformView(decoded, callback);
        return;
    }
    callback(null);
  }
}
