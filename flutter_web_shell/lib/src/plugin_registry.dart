import 'dart:async';
import 'dart:ui';

import 'package:flutter/services.dart';

typedef _MessageHandler = Future<ByteData> Function(ByteData);

class PluginRegistry {
  Registrar registrarFor(Type key) => Registrar();
}

class Registrar {
  BinaryMessenger get messenger => _platformBinaryMessenger;
}

final shellPluginRegistry = PluginRegistry();

class _PlatformBinaryMessenger extends BinaryMessenger {
  final Map<String, _MessageHandler> _handlers = <String, _MessageHandler>{};
  final Map<String, _MessageHandler> _mockHandlers = <String, _MessageHandler>{};

  @override
  Future<void> handlePlatformMessage(
      String channel, ByteData data, PlatformMessageResponseCallback callback) {
    // TODO: implement handlePlatformMessage
    return null;
  }

  /// Sends a platform message from the platform side back to the framework.
  @override
  Future<ByteData> send(String channel, ByteData message) {
    // TODO: implement send
    return null;
  }

  @override
  void setMessageHandler(
      String channel, Future<ByteData> Function(ByteData message) handler) {
    if (handler == null)
      _handlers.remove(channel);
    else
      _handlers[channel] = handler;
  }

  @override
  void setMockMessageHandler(
      String channel, Future<ByteData> Function(ByteData message) handler) {
    if (handler == null)
      _mockHandlers.remove(channel);
    else
      _mockHandlers[channel] = handler;
  }
}

final _PlatformBinaryMessenger _platformBinaryMessenger =
    _PlatformBinaryMessenger();
