// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:collection';
import 'dart:typed_data';

import 'package:zircon/zircon.dart';
import 'package:meta/meta.dart';

import 'error.dart';
import 'interface.dart';
import 'message.dart';

/// The different states that an [AsyncBinding] or [AsyncProxy] can be in.
enum InterfaceState {
  /// The binding or proxy has not yet been bound.
  unbound,

  /// The binding or proxy has been bound to a channel.
  bound,

  /// The binding or proxy has been closed.
  closed
}

/// An exception that's thrown if an [AsyncBinding] or [AsyncProxy] isn't in the required
/// state for the requested operation.
class FidlStateException extends FidlError {
  /// Create a new [FidlStateException].
  FidlStateException(String message) : super(message);
}

/// A class that holds and mutates the state of an [AsyncBinding] or [AsyncProxy],
/// represented as an [InterfaceState] value.
abstract class _Stateful {
  InterfaceState _currentState = InterfaceState.unbound;

  /// The controller for the stream of state changes.
  final StreamController<InterfaceState> _streamController =
      StreamController.broadcast();

  /// Gets the current state.
  InterfaceState get state => _currentState;

  /// Change the state.
  @protected
  set state(InterfaceState newState) {
    if (_currentState == newState) {
      // No change.
      return;
    }
    if (newState.index < _currentState.index) {
      throw FidlStateException(
          "Can't change InterfaceState from $_currentState to $newState.");
    }
    _currentState = newState;
    _streamController.add(newState);
  }

  /// A stream of state changes.
  Stream<InterfaceState> get stateChanges => _streamController.stream;

  /// Is this interface unbound?
  bool get isUnbound => _currentState == InterfaceState.unbound;

  /// Is this interface bound?
  bool get isBound => _currentState == InterfaceState.bound;

  /// A future that completes when the interface becomes bound.
  Future<void> get whenBound {
    if (_currentState == InterfaceState.unbound) {
      return stateChanges.firstWhere((s) => s == InterfaceState.bound);
    }
    if (_currentState == InterfaceState.bound) {
      return Future.value();
    }
    return Future.error(
        FidlStateException('Interface will never become bound'));
  }

  /// Is this interface closed?
  bool get isClosed => _currentState == InterfaceState.closed;

  /// A future that completes when the interface is closed.
  Future<void> get whenClosed {
    if (_currentState == InterfaceState.closed) {
      return Future.value();
    }
    return stateChanges.firstWhere((s) => s == InterfaceState.closed);
  }
}

/// Listens for messages and dispatches them to an implementation of [T].
abstract class AsyncBinding<T> extends _Stateful {
  /// Creates a binding object in an unbound state.
  ///
  /// Rather than creating a [AsyncBinding<T>] object directly, you typically create
  /// a `TBinding` object, which are subclasses of [AsyncBinding<T>] created by the
  /// FIDL compiler for a specific interface.
  AsyncBinding(this.$interfaceName) {
    _reader
      ..onReadable = _handleReadable
      ..onError = _handleError;
  }

  /// The name of the interface [T] as a string.
  ///
  /// This is used to generate meaningful error messages at runtime.
  final String $interfaceName;

  /// Returns an interface handle whose peer is bound to the given object.
  ///
  /// Creates a channel pair, binds one of the channels to this object, and
  /// returns the other channel. Messages sent over the returned channel will be
  /// decoded and dispatched to `impl`.
  ///
  /// The `impl` parameter must not be null.
  InterfaceHandle<T> wrap(T impl) {
    if (!isUnbound) {
      throw FidlStateException("AsyncBinding<${$interfaceName}> isn't unbound");
    }
    ChannelPair pair = ChannelPair();
    if (pair.status != ZX.OK) {
      throw Exception(
          "AsyncBinding<${$interfaceName}> couldn't create channel: ${getStringForStatus(pair.status)}");
    }
    _impl = impl;
    _reader.bind(pair.first!);

    state = InterfaceState.bound;

    return InterfaceHandle<T>(pair.second);
  }

  /// Binds the given implementation to the given interface request.
  ///
  /// Listens for messages on channel underlying the given interface request,
  /// decodes them, and dispatches the decoded messages to `impl`.
  ///
  /// This object must not already be bound.
  ///
  /// The `impl` and `interfaceRequest` parameters must not be `null`. The
  /// `channel` property of the given `interfaceRequest` must not be `null`.
  ///
  /// Implementation note: in a generic context, the inferred type when creating
  /// the `interfaceRequest` may be a super-class of `T`, possibly `Service`.
  /// Since concrete classes of `AsyncBinding` are code generated, the type
  /// parameter `T` will always be precise, even when used in a generic context.
  /// As a result, we cannot type-bound `interfaceRequest` statically, and
  /// should instead rely on dynamic behavior.
  void bind(T impl, InterfaceRequest<dynamic> interfaceRequest) {
    if (!isUnbound) {
      throw FidlStateException("AsyncBinding<${$interfaceName}> isn't unbound");
    }
    if (impl == null) {
      throw FidlError(
          "AsyncBinding<${$interfaceName}> can't bind to a null impl");
    }

    Channel? channel = interfaceRequest.passChannel();
    if (channel == null) {
      throw FidlError(
          "AsyncBinding<${$interfaceName}> can't bind to a null InterfaceRequest channel");
    }

    _impl = impl;
    _reader.bind(channel);

    state = InterfaceState.bound;
  }

  /// Unbinds [impl] and returns the unbound channel as an interface request.
  ///
  /// Stops listening for messages on the bound channel, wraps the channel in an
  /// interface request of the appropriate type, and returns that interface
  /// request.
  ///
  /// The object must have previously been bound (e.g., using [bind]).
  InterfaceRequest<T> unbind() {
    if (!isBound) {
      throw FidlStateException("AsyncBinding<${$interfaceName}> isn't bound");
    }
    final InterfaceRequest<T> result = InterfaceRequest<T>(_reader.unbind());
    _impl = null;

    state = InterfaceState.closed;

    return result;
  }

  void _writeEpitaph(int statusCode) {
    var bytes = ByteData(24)
      ..setUint32(0, 0) // txid (0 because not tracking this on server side)
      ..setUint8(4, 0) // flags
      ..setUint8(5, 0) // flags
      ..setUint8(6, 0) // flags
      ..setUint8(7, 0) // magic byte
      ..setUint64(8, epitaphOrdinal) // ordinal
      ..setInt32(16, statusCode); // body (epitaph struct)
    _reader.channel?.write(bytes, []);
  }

  /// Close the bound channel.
  ///
  /// This function does nothing if the object is not bound.
  void close([int? statusCode]) {
    if (isBound) {
      if (statusCode != null) {
        _writeEpitaph(statusCode);
      }
      _reader.close();
      _impl = null;

      state = InterfaceState.closed;
    }
  }

  /// The implementation of [T] bound using this object.
  ///
  /// If this object is not bound, this property is null.
  T? get impl => _impl;
  T? _impl;

  /// Decodes the given message and dispatches the decoded message to [impl].
  ///
  /// This function is called by this object whenever a message arrives over a
  /// bound channel.
  @protected
  void handleMessage(IncomingMessage message, OutgoingMessageSink respond);

  void _handleReadable() {
    final ReadResult result = _reader.channel!.queryAndRead();
    if (result.bytes.lengthInBytes == 0) {
      throw FidlError(
          'AsyncBinding<${$interfaceName}> Unexpected empty message or error: $result');
    }

    final IncomingMessage message = IncomingMessage.fromReadResult(result);
    if (!message.isCompatible()) {
      close();
      throw FidlError(
          'Incompatible wire format', FidlErrorCode.fidlUnknownMagic);
    }
    handleMessage(message, sendMessage);
  }

  /// Always called when the channel underneath closes.
  void _handleError(ChannelReaderError error) {
    /// TODO(ianloic): do something with [error].
    close();
  }

  /// Sends the given message over the bound channel.
  ///
  /// If the channel is not bound, the handles inside the message are closed and
  /// the message itself is discarded.
  void sendMessage(OutgoingMessage response) {
    if (!isBound) {
      response.closeHandles();
      return;
    }
    _reader.channel?.write(response.data, response.handles);
  }

  final ChannelReader _reader = ChannelReader();
}

/// Representation of a service that all [T] implementations should extend from.
abstract class Service {
  /// Getter for the [ServiceData]
  ServiceData? get $serviceData;
}

/// Exposes the ability to get a hold of the service runtime name and bindings.
abstract class ServiceData<T> {
  /// Returns the generated runtime service name.
  String getName();

  /// Returns the generated runtime service bindings.
  AsyncBinding getBinding();
}

/// Sends messages to a remote implementation of [T]
class AsyncProxy<T> {
  /// Creates a proxy object with the given [ctrl].
  ///
  /// Rather than creating [Proxy<T>] object directly, you typically create
  /// `TProxy` objects, which are subclasses of [Proxy<T>] created by the FIDL
  /// compiler for a specific interface.
  AsyncProxy(this.ctrl);

  /// The control plane for this proxy.
  ///
  /// Methods that manipulate the local proxy (as opposed to sending messages
  /// to the remote implementation of [T]) are exposed on this [ctrl] object to
  /// avoid naming conflicts with the methods of [T].
  final AsyncProxyController<T> ctrl;

  // In general it's probably better to avoid adding fields and methods to this
  // class. Names added to this class have to be mangled by bindings generation
  // to avoid name conflicts.
}

/// A controller for Future based proxies.
class AsyncProxyController<T> extends _Stateful {
  final ChannelReader _reader = ChannelReader();

  final HashMap<int, Completer<dynamic>> _completerMap = HashMap();
  int _nextTxid = 1;

  /// Creates proxy controller.
  ///
  /// Proxy controllers are not typically created directly. Instead, you
  /// typically obtain an [AsyncProxyController<T>] object as the [AsyncProxy<T>.ctrl]
  /// property of a `TProxy` object.
  AsyncProxyController({this.$serviceName, this.$interfaceName}) {
    _reader
      ..onReadable = _handleReadable
      ..onError = _handleError;
    whenClosed.then((_) {
      for (final Completer completer in _completerMap.values) {
        if (!completer.isCompleted) {
          completer.completeError(FidlError(
              'AsyncProxyController<${$interfaceName}> connection closed'));
        }
      }
      _completerMap.clear();
    }, onError: (_) {
      // Ignore errors.
    });
  }

  /// The service name associated with [T], if any.
  ///
  /// Will be set if the `[Discoverable]` attribute is on the FIDL interface
  /// definition. If set it will be the fully-qualified name of the interface.
  ///
  /// This string is typically used with the `ServiceProvider` interface to
  /// request an implementation of [T].
  final String? $serviceName;

  /// The name of the interface of [T].
  ///
  /// Unlike [$serviceName] should always be set and won't be fully qualified.
  /// This should only be used for debugging and logging purposes.
  final String? $interfaceName;

  /// Creates an interface request whose peer is bound to this interface proxy.
  ///
  /// Creates a channel pair, binds one of the channels to this object, and
  /// returns the other channel. Calls to the proxy will be encoded as messages
  /// and sent to the returned channel.
  ///
  /// The proxy must not already have been bound.
  InterfaceRequest<T> request() {
    if (!isUnbound) {
      throw FidlStateException(
          "AsyncProxyController<${$interfaceName}> isn't unbound");
    }

    ChannelPair pair = ChannelPair();
    if (pair.status != ZX.OK) {
      throw FidlError(
          "AsyncProxyController<${$interfaceName}> couldn't create channel: ${getStringForStatus(pair.status)}");
    }
    _reader.bind(pair.first!);
    state = InterfaceState.bound;

    return InterfaceRequest<T>(pair.second);
  }

  /// Binds the proxy to the given interface handle.
  ///
  /// Calls to the proxy will be encoded as messages and sent over the channel
  /// underlying the given interface handle.
  ///
  /// This object must not already be bound.
  ///
  /// The `interfaceHandle` parameter must not be null. The `channel` property
  /// of the given `interfaceHandle` must not be null.
  void bind(InterfaceHandle<T> interfaceHandle) {
    if (!isUnbound) {
      throw FidlStateException(
          "AsyncProxyController<${$interfaceName}> isn't unbound");
    }

    final channel = interfaceHandle.passChannel();
    if (channel == null) {
      throw FidlError(
          "AsyncProxyController<${$interfaceName}> can't bind to null InterfaceHandle channel");
    }

    _reader.bind(channel);
    state = InterfaceState.bound;
  }

  /// Unbinds the proxy and returns the unbound channel as an interface handle.
  ///
  /// Calls on the proxy will no longer be encoded as messages on the bound
  /// channel.
  ///
  /// The proxy must have previously been bound (e.g., using [bind]).
  InterfaceHandle<T> unbind() {
    if (!isBound) {
      throw FidlStateException(
          "AsyncProxyController<${$interfaceName}> isn't bound");
    }
    if (!_reader.isBound) {
      throw FidlError(
          "AsyncProxyController<${$interfaceName}> reader isn't bound");
    }

    state = InterfaceState.closed;

    return InterfaceHandle<T>(_reader.unbind());
  }

  /// Close the channel bound to the proxy.
  ///
  /// The proxy must have previously been bound (e.g., using [bind]).
  void close() {
    _close(null);
  }

  void _close(FidlError? error) {
    if (isBound) {
      _reader.close();
      state = InterfaceState.closed;
      _completerMap.forEach((_, Completer<dynamic> completer) =>
          completer.completeError(FidlStateException(error != null
              ? 'AsyncProxyController<${$interfaceName}> is closed with error: ${error.message}'
              : 'AsyncProxyController<${$interfaceName}> is closed.')));
    }
  }

  ///  close the channel and forwards error to any open completers.
  void proxyError(FidlError error) {
    _close(error);
  }

  /// Called when an epitaph is received (from channel closure).
  EpitaphHandler? onEpitaphReceived;

  /// Called whenever this object receives a response on a bound channel.
  ///
  /// Used by subclasses of [Proxy<T>] to receive responses to messages.
  IncomingMessageSink? onResponse;

  void _handleReadable() {
    final ReadResult result = _reader.channel!.queryAndRead();
    if (result.bytes.lengthInBytes == 0) {
      proxyError(FidlError(
          'AsyncProxyController<${$interfaceName}>: Read from channel failed'));
      return;
    }
    try {
      IncomingMessage message = IncomingMessage.fromReadResult(result);
      final epitaphCallback = onEpitaphReceived;
      final responseCallback = onResponse;
      if (message.ordinal == epitaphOrdinal) {
        int statusCode = message.data.getInt32(16);
        if (epitaphCallback != null) {
          epitaphCallback(statusCode);
        }
      } else if (responseCallback != null) {
        responseCallback(message);
      }
    } on FidlError catch (e) {
      for (Handle handle in result.handles) {
        handle.close();
      }
      proxyError(e);
    }
  }

  /// Always called when the channel underneath closes.
  void _handleError(ChannelReaderError error) {
    proxyError(FidlError(error.toString()));
  }

  /// Sends the given messages over the bound channel.
  ///
  /// Used by subclasses of [Proxy<T>] to send encoded messages.
  void sendMessage(OutgoingMessage message) {
    if (!_reader.isBound) {
      proxyError(FidlStateException(
          'AsyncProxyController<${$interfaceName}> is closed.'));
      return;
    }
    final int status = _reader.channel!.write(message.data, message.handles);
    if (status != ZX.OK) {
      proxyError(FidlError(
          'AsyncProxyController<${$interfaceName}> failed to write to channel: ${_reader.channel} (status: $status)'));
    }
  }

  /// Sends the given messages over the bound channel and registers a Completer
  /// to handle the response.
  ///
  /// Used by subclasses of [AsyncProxy<T>] to send encoded messages.
  void sendMessageWithResponse(
      OutgoingMessage message, Completer<dynamic> completer) {
    if (!_reader.isBound) {
      proxyError(FidlStateException(
          'AsyncProxyController<${$interfaceName}> is closed.'));
      return;
    }

    const int _userspaceTxidMask = 0x7FFFFFFF;

    int txid = _nextTxid++ & _userspaceTxidMask;
    while (txid == 0 || _completerMap.containsKey(txid))
      txid = _nextTxid++ & _userspaceTxidMask;
    message.txid = txid;
    _completerMap[message.txid] = completer;
    final int status = _reader.channel!.write(message.data, message.handles);

    if (status != ZX.OK) {
      proxyError(FidlError(
          'AsyncProxyController<${$interfaceName}> failed to write to channel: ${_reader.channel} (status: $status)'));
      return;
    }
  }

  /// Returns the completer associated with the given response message.
  ///
  /// Used by subclasses of [AsyncProxy<T>] to retrieve registered completers when
  /// handling response messages.
  Completer? getCompleter(int txid) {
    final Completer? result = _completerMap.remove(txid);
    if (result == null) {
      proxyError(FidlError('Message had unknown request id: $txid'));
    }
    return result;
  }
}
