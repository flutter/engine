part of engine;

// import 'dart:html' as html;

// import 'dart:typed_data';

// import '../../engine.dart' show MethodCodec, MethodCall, StandardMethodCodec;
// import '../../../ui.dart' as ui show PlatformMessageResponseCallback;
// import './content_manager.dart';

typedef ContentSlotHandler = void Function(int, html.SlotElement);
typedef ContentViewHandler = void Function(int, html.Element);

// This class 
class PlatformViewMessageHandler {
  final MethodCodec _codec = StandardMethodCodec();

  final ContentSlotHandler viewSlotHandler;
  final ContentViewHandler viewContentHandler;
  final PlatformViewContentManager contentManager;

  PlatformViewMessageHandler({
    required this.contentManager, required this.viewSlotHandler, required this.viewContentHandler
  });

  void _createPlatformView(
      MethodCall methodCall, ui.PlatformMessageResponseCallback callback) {

    final Map<dynamic, dynamic> args = methodCall.arguments;
    final int viewId = args['id'];
    final String viewType = args['viewType'];

    if (!contentManager.knowsViewType(viewType)) {
      callback(_codec.encodeErrorEnvelope(
        code: 'Unregistered factory',
        message: "No factory registered for viewtype '$viewType'",
      ));
      return;
    }

    final html.Element content = contentManager.renderContent(viewType, viewId, args);
    final html.SlotElement slot = contentManager.renderSlot(viewType, viewId);

    try {
      viewContentHandler(viewId, content);
      viewSlotHandler(viewId, slot);

      callback(_codec.encodeSuccessEnvelope(null));
    } catch (e) {
      callback(_codec.encodeErrorEnvelope(
        code: 'Failure rendering platform view',
        message: e.toString(),
      ));
    }
  }

  void _disposePlatformView(
      MethodCall methodCall, ui.PlatformMessageResponseCallback callback) {

    final int viewId = methodCall.arguments;

    contentManager.clearPlatformView(viewId);

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
