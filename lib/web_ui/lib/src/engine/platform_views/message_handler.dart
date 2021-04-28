// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

// import 'dart:html' as html;

// import 'dart:typed_data';

// import '../../engine.dart' show MethodCodec, MethodCall, StandardMethodCodec;
// import '../../../ui.dart' as ui show PlatformMessageResponseCallback;
// import './content_manager.dart';

/// A function that handles a newly created [html.Element] for a platform
/// view SLOT (insertion point) with a unique [int] id.
typedef PlatformViewSlotHandler = void Function(int, html.Element);

/// A function that handle a newly created [html.Element] with the contents of a
/// platform view with a unique [int] id.
typedef PlatformViewContentHandler = void Function(int, html.Element);

/// A function that handles part of the `dispose` lifecycle of a platform view
/// with a unique [int] id.
typedef PlatformViewDisposeHandler = void Function(int);

/// This class handles incoming framework messages to create/dispose Platform Views.
///
/// (An instance of this class is connected to the `flutter/platform_views`
/// Platform Channel in the [EnginePlatformDispatcher] class.)
///
/// It uses a [PlatformViewManager] to handle the CRUD of the DOM of Platform Views.
/// This `contentManager` is shared across the engine, to perform
/// all operations related to platform views (registration, rendering, etc...),
/// regardless of the rendering backend.
///
/// When a platform view is created, the injection into the App DOM is delegated
/// to a [PlatformViewSlotHandler] and a [PlatformViewContentHandler] function,
/// which will decide where in the global DOM to inject the different parts
/// of the Platform View.
///
/// When a Platform View is disposed of, it is removed from the cache (and DOM)
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

  /// Handle a `create` Platform View message.
  ///
  /// This will attempt to render the `contents` and `slot` of a Platform View,
  /// if its `viewType` has been registered previously.
  ///
  /// (See [PlatformViewContentManager.registerFactory] for more details.)
  ///
  /// When the `contents` and `slot` of the Platform View are created, they are
  /// delegated to the [_slotHandler] and [_contentHandler] functions, so the
  /// active rendering backend can inject them in the right place of the DOM.
  ///
  /// If all goes well, this function will `callback` with an empty success envelope.
  /// In case of error, this will `callback` with an error envelope describing the error.
  void _createPlatformView(
    MethodCall methodCall,
    ui.PlatformMessageResponseCallback callback,
  ) {
    final Map<dynamic, dynamic> args = methodCall.arguments;
    final int viewId = args['id'];
    final String viewType = args['viewType'];

    if (!_contentManager.knowsViewType(viewType)) {
      callback(_codec.encodeErrorEnvelope(
        code: 'unregistered_view_type',
        message: 'trying to create a view with an unregistered type',
        details: 'unregistered view type: $viewType',
      ));
      return;
    }

    final html.Element content = _contentManager.renderContent(
      viewType,
      viewId,
      args,
    );
    final html.Element slot = _contentManager.renderSlot(viewType, viewId);

    try {
      // For now, we don't need anything fancier. If needed, this can be converted
      // to a PlatformViewStrategy class for each web-renderer backend?
      if (_contentHandler != null) {
        _contentHandler!(viewId, content);
      }
      if (_slotHandler != null) {
        _slotHandler!(viewId, slot);
      }

      callback(_codec.encodeSuccessEnvelope(null));
    } on _PlatformViewAlreadyCreatedException catch (e) {
      callback(_codec.encodeErrorEnvelope(
        code: 'recreating_view',
        message: 'trying to create an already created view',
        details: 'view id: ${e.viewId}',
      ));
    }
  }

  /// Handle a `dispose` Platform View message.
  ///
  /// This will clear the cached information that the framework has about a given
  /// `viewId`, through the [_contentManager].
  ///
  /// Once that's done, the dispose call is delegated to the [_disposeHandler]
  /// function, so the active rendering backend can dispose of whatever resources
  /// it needed to get ahold of.
  ///
  /// This function should always `callback` with an empty success envelope.
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

  /// Handles a PlatformViewCall to the `flutter/platform_views` channel.
  ///
  /// This method handles two possible messages:
  /// * `create`: See [_createPlatformView]
  /// * `dispose`: See [_disposePlatformView]
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
