// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:collection';

import 'package:meta/meta.dart';
import 'package:zircon/zircon.dart';

import 'error.dart';
import 'message.dart';

// ignore_for_file: public_member_api_docs

const epitaphOrdinal = 0xffffffffffffffff;

typedef _VoidCallback = void Function();

typedef EpitaphHandler = void Function(int statusCode);

/// A channel over which messages from interface T can be sent.
///
/// An interface handle holds a [channel] whose peer expects to receive messages
/// from the FIDL interface T. The channel held by an interface handle is not
/// currently bound, which means messages cannot yet be exchanged with the
/// channel's peer.
///
/// To send messages over the channel, bind the interface handle to a `TProxy`
/// object using use [ProxyController<T>.bind] method on the proxy's
/// [Proxy<T>.ctrl] property.
///
/// Example:
///
/// ```dart
/// InterfaceHandle<T> fooHandle = [...]
/// FooProxy foo = new FooProxy();
/// foo.ctrl.bind(fooHandle);
/// foo.bar();
/// ```
///
/// To obtain an interface handle to send over a channel, used the
/// [Binding<T>.wrap] method on an object of type `TBinding`.
///
/// Example:
///
/// ```dart
/// class FooImpl extends Foo {
///   final FooBinding _binding = new FooBinding();
///
///   InterfaceHandle<T> getInterfaceHandle() => _binding.wrap(this);
///
///   @override
///   void bar() {
///     print('Received bar message.');
///   }
/// }
/// ```
class InterfaceHandle<T> {
  /// Creates an interface handle that wraps the given channel.
  InterfaceHandle(this._channel);

  /// The underlying channel messages will be sent over when the interface
  /// handle is bound to a [Proxy].
  ///
  /// To take the channel from this object, use [passChannel].
  Channel? get channel => _channel;
  Channel? _channel;

  /// Returns [channel] and sets [channel] to `null`.
  ///
  /// Useful for taking ownership of the underlying channel.
  Channel? passChannel() {
    final Channel? result = _channel;
    _channel = null;
    return result;
  }

  /// Closes the underlying channel.
  void close() {
    _channel?.close();
    _channel = null;
  }
}

/// A channel over which messages from interface T can be received.
///
/// An interface request holds a [channel] whose peer expects to be able to send
/// messages from the FIDL interface T. A channel held by an interface request
/// is not currently bound, which means messages cannot yet be exchanged with
/// the channel's peer.
///
/// To receive messages sent over the channel, bind the interface handle using
/// [Binding<T>.bind] on a `TBinding` object, which you typically hold as a
/// private member variable in a class that implements [T].
///
/// Example:
///
/// ```dart
/// class FooImpl extends Foo {
///   final FooBinding _binding = new FooBinding();
///
///   void bind(InterfaceRequest<T> request) {
///     _binding.bind(request);
///   }
///
///   @override
///   void bar() {
///     print('Received bar message.');
///   }
/// }
/// ```
///
/// To obtain an interface request to send over a channel, used the
/// [ProxyController<T>.request] method on the [Proxy<T>.ctrl] property of an
/// object of type `TProxy`.
///
/// Example:
///
/// ```dart
/// FooProxy foo = new FooProxy();
/// InterfaceRequest<T> request = foo.ctrl.request();
/// ```
class InterfaceRequest<T> {
  /// Creates an interface request that wraps the given channel.
  InterfaceRequest(this._channel);

  /// The underlying channel messages will be received over when the interface
  /// handle is bound to [Binding].
  ///
  /// To take the channel from this object, use [passChannel].
  Channel? get channel => _channel;
  Channel? _channel;

  /// Returns [channel] and sets [channel] to `null`.
  ///
  /// Useful for taking ownership of the underlying channel.
  Channel? passChannel() {
    final Channel? result = _channel;
    _channel = null;
    return result;
  }

  /// Closes the underlying channel.
  void close() {
    _channel?.close();
    _channel = null;
  }
}

class InterfacePair<T> {
  InterfacePair() {
    ChannelPair pair = ChannelPair();
    request = InterfaceRequest<T>(pair.first);
    handle = InterfaceHandle<T>(pair.second);
  }

  InterfaceRequest<T>? request;
  InterfaceHandle<T>? handle;

  InterfaceRequest<T>? passRequest() {
    final InterfaceRequest<T>? result = request;
    request = null;
    return result;
  }

  InterfaceHandle<T>? passHandle() {
    final InterfaceHandle<T>? result = handle;
    handle = null;
    return result;
  }
}

/// Listens for messages and dispatches them to an implementation of T.
abstract class Binding<T> {
  /// Creates a binding object in an unbound state.
  ///
  /// Rather than creating a [Binding<T>] object directly, you typically create
  /// a `TBinding` object, which are subclasses of [Binding<T>] created by the
  /// FIDL compiler for a specific interface.
  Binding() {
    _reader
      ..onReadable = _handleReadable
      ..onError = _handleError;
  }

  /// Event for binding.
  _VoidCallback? onBind;

  /// Event for unbinding.
  _VoidCallback? onUnbind;

  /// Event for when the binding is closed.
  _VoidCallback? onClose;

  /// Returns an interface handle whose peer is bound to the given object.
  ///
  /// Creates a channel pair, binds one of the channels to this object, and
  /// returns the other channel. Messages sent over the returned channel will be
  /// decoded and dispatched to `impl`.
  ///
  /// The `impl` parameter must not be null.
  InterfaceHandle<T>? wrap(T impl) {
    assert(!isBound);
    ChannelPair pair = ChannelPair();
    if (pair.status != ZX.OK) {
      return null;
    }
    _impl = impl;
    _reader.bind(pair.first!);

    final callback = onBind;
    if (callback != null) {
      callback();
    }

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
  void bind(T impl, InterfaceRequest<T> interfaceRequest) {
    assert(!isBound);
    Channel channel = interfaceRequest.passChannel()!;
    _impl = impl;
    _reader.bind(channel);

    final callback = onBind;
    if (callback != null) {
      callback();
    }
  }

  /// Unbinds [impl] and returns the unbound channel as an interface request.
  ///
  /// Stops listening for messages on the bound channel, wraps the channel in an
  /// interface request of the appropriate type, and returns that interface
  /// request.
  ///
  /// The object must have previously been bound (e.g., using [bind]).
  InterfaceRequest<T> unbind() {
    assert(isBound);
    final InterfaceRequest<T> result = InterfaceRequest<T>(_reader.unbind());
    _impl = null;

    final callback = onUnbind;
    if (callback != null) {
      callback();
    }

    return result;
  }

  /// Close the bound channel.
  ///
  /// This function does nothing if the object is not bound.
  void close() {
    if (isBound) {
      _reader.close();
      _impl = null;

      final callback = onClose;
      if (callback != null) {
        callback();
      }
    }
  }

  /// Called when the channel underneath closes.
  _VoidCallback? onConnectionError;

  /// The implementation of [T] bound using this object.
  ///
  /// If this object is not bound, this property is null.
  T? get impl => _impl;
  T? _impl;

  /// Whether this object is bound to a channel.
  ///
  /// See [bind] and [unbind] for more information.
  bool get isBound => _impl != null;

  /// Decodes the given message and dispatches the decoded message to [impl].
  ///
  /// This function is called by this object whenever a message arrives over a
  /// bound channel.
  @protected
  void handleMessage(IncomingMessage message, OutgoingMessageSink respond);

  void _handleReadable() {
    final ReadResult result = _reader.channel!.queryAndRead();
    if (result.bytes.lengthInBytes == 0) {
      throw FidlError('Unexpected empty message or error: $result '
          'from channel ${_reader.channel}');
    }

    final IncomingMessage message = IncomingMessage.fromReadResult(result);
    if (!message.isCompatible()) {
      close();
      throw FidlError(
          'Incompatible wire format', FidlErrorCode.fidlUnknownMagic);
    }
    handleMessage(message, sendMessage);
  }

  /// Always called when the channel underneath closes. If [onConnectionError]
  /// is set, it is called.
  void _handleError(ChannelReaderError error) {
    final callback = onConnectionError;
    if (callback != null) {
      callback();
    }
  }

  /// Sends the given message over the bound channel.
  ///
  /// If the channel is not bound, the handles inside the message are closed and
  /// the message itself is discarded.
  void sendMessage(OutgoingMessage response) {
    if (!_reader.isBound) {
      response.closeHandles();
      return;
    }
    _reader.channel!.write(response.data, response.handles);
  }

  final ChannelReader _reader = ChannelReader();
}

/// The object that [ProxyController<T>.error] completes with when there is
/// an error.
class ProxyError {
  /// Creates a proxy error with the given message.
  ///
  /// The `message` argument must not be null.
  ProxyError(this.message);

  /// What went wrong.
  final String message;

  @override
  String toString() => 'ProxyError: $message';
}

/// The control plane for an interface proxy.
///
/// A proxy controller lets you operate on the local [Proxy<T>] object itself
/// rather than send messages to the remote implementation of the proxy. For
/// example, you can [unbind] or [close] the proxy.
///
/// You typically obtain a [ProxyController<T>] object as the [Proxy<T>.ctrl]
/// property of a `TProxy` object.
///
/// Example:
///
/// ```dart
/// FooProxy foo = new FooProxy();
/// fooProvider.getFoo(foo.ctrl.request());
/// ```
class ProxyController<T> {
  /// Creates proxy controller.
  ///
  /// Proxy controllers are not typically created directly. Instead, you
  /// typically obtain a [ProxyController<T>] object as the [Proxy<T>.ctrl]
  /// property of a `TProxy` object.
  ProxyController({this.$serviceName, this.$interfaceName}) {
    _reader
      ..onReadable = _handleReadable
      ..onError = _handleError;
  }

  /// Event for binding.
  _VoidCallback? onBind;

  /// Event for unbinding.
  _VoidCallback? onUnbind;

  /// Event for when the binding is closed.
  _VoidCallback? onClose;

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
    assert(!isBound);
    ChannelPair pair = ChannelPair();
    assert(pair.status == ZX.OK);
    _reader.bind(pair.first!);

    _boundCompleter.complete();
    final callback = onBind;
    if (callback != null) {
      callback();
    }

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
    assert(!isBound);
    _reader.bind(interfaceHandle.passChannel()!);

    _boundCompleter.complete();
    final callback = onBind;
    if (callback != null) {
      callback();
    }
  }

  /// Unbinds the proxy and returns the unbound channel as an interface handle.
  ///
  /// Calls on the proxy will no longer be encoded as messages on the bound
  /// channel.
  ///
  /// The proxy must have previously been bound (e.g., using [bind]).
  InterfaceHandle<T>? unbind() {
    assert(isBound);
    if (!_reader.isBound) {
      return null;
    }

    final callback = onUnbind;
    if (callback != null) {
      callback();
    }

    // TODO(rosswang): Do we need to _reset() here?
    return InterfaceHandle<T>(_reader.unbind());
  }

  /// Whether this object is bound to a channel.
  ///
  /// See [bind] and [unbind] for more information.
  bool get isBound => _reader.isBound;

  /// Close the channel bound to the proxy.
  ///
  /// The proxy must have previously been bound (e.g., using [bind]).
  void close() {
    if (isBound) {
      if (_pendingResponsesCount > 0) {
        proxyError('The proxy is closed.');
      }
      _reset();
      _reader.close();

      final callback = onClose;
      if (callback != null) {
        callback();
      }
    }
  }

  /// Called when the channel underneath closes.
  _VoidCallback? onConnectionError;

  /// Called whenever this object receives a response on a bound channel.
  ///
  /// Used by subclasses of [Proxy<T>] to receive responses to messages.
  IncomingMessageSink? onResponse;

  final ChannelReader _reader = ChannelReader();
  final HashMap<int, Function> _callbackMap = HashMap<int, Function>();

  /// A future that completes when an error is generated by the proxy.
  Future<ProxyError> get error => _errorCompleter.future;
  Completer<ProxyError> _errorCompleter = Completer<ProxyError>();

  /// A future that completes when the proxy is bound.
  Future<Null> get bound => _boundCompleter.future;
  Completer<Null> _boundCompleter = Completer<Null>();

  int _nextTxid = 1;
  int _pendingResponsesCount = 0;

  void _reset() {
    _callbackMap.clear();
    _errorCompleter = Completer<ProxyError>();
    if (!_boundCompleter.isCompleted) {
      _boundCompleter.completeError('Proxy<${$interfaceName}> closed.');
    }
    _boundCompleter = Completer<Null>();
    _nextTxid = 1;
    _pendingResponsesCount = 0;
  }

  void _handleReadable() {
    final ReadResult result = _reader.channel!.queryAndRead();
    if (result.bytes.lengthInBytes == 0) {
      proxyError('Read from channel ${_reader.channel} failed');
      return;
    }
    try {
      _pendingResponsesCount--;
      final callback = onResponse;
      if (callback != null) {
        callback(IncomingMessage.fromReadResult(result));
      }
    } on FidlError catch (e) {
      for (Handle handle in result.handles) {
        handle.close();
      }
      proxyError(e.toString());
      close();
    }
  }

  /// Always called when the channel underneath closes. If [onConnectionError]
  /// is set, it is called.
  void _handleError(ChannelReaderError error) {
    proxyError(error.toString());
    _reset();
    final callback = onConnectionError;
    if (callback != null) {
      callback();
    }
  }

  /// Sends the given messages over the bound channel.
  ///
  /// Used by subclasses of [Proxy<T>] to send encoded messages.
  void sendMessage(OutgoingMessage message) {
    if (!_reader.isBound) {
      proxyError('The proxy is closed.');
      return;
    }
    final int status = _reader.channel!.write(message.data, message.handles);
    if (status != ZX.OK)
      proxyError(
          'Failed to write to channel: ${_reader.channel} (status: $status)');
  }

  /// Sends the given messages over the bound channel and registers a callback
  /// to handle the response.
  ///
  /// Used by subclasses of [Proxy<T>] to send encoded messages.
  void sendMessageWithResponse(OutgoingMessage message, Function callback) {
    if (!_reader.isBound) {
      proxyError('The sender is closed.');
      return;
    }

    const int _kUserspaceTxidMask = 0x7FFFFFFF;

    int txid = _nextTxid++ & _kUserspaceTxidMask;
    while (txid == 0 || _callbackMap.containsKey(txid))
      txid = _nextTxid++ & _kUserspaceTxidMask;
    message.txid = txid;
    final int status = _reader.channel!.write(message.data, message.handles);

    if (status != ZX.OK) {
      proxyError(
          'Failed to write to channel: ${_reader.channel} (status: $status)');
      return;
    }

    _callbackMap[message.txid] = callback;
    _pendingResponsesCount++;
  }

  /// Returns the callback associated with the given response message.
  ///
  /// Used by subclasses of [Proxy<T>] to retrieve registered callbacks when
  /// handling response messages.
  Function? getCallback(int txid) {
    final Function? result = _callbackMap.remove(txid);
    if (result == null) {
      proxyError('Message had unknown request id: $txid');
      return null;
    }
    return result;
  }

  /// Complete the [error] future with the given message.
  void proxyError(String message) {
    final fullMessage =
        'Error in proxy with interface name [${$interfaceName}] and '
        'service name [${$serviceName}]: $message';
    print(fullMessage);
    if (!_errorCompleter.isCompleted) {
      error.whenComplete(() {
        _errorCompleter = Completer<ProxyError>();
      });
      _errorCompleter.complete(ProxyError(fullMessage));
    }
  }
}

/// Sends messages to a remote implementation of [T]
class Proxy<T> {
  /// Creates a proxy object with the given [ctrl].
  ///
  /// Rather than creating [Proxy<T>] object directly, you typically create
  /// `TProxy` objects, which are subclasses of [Proxy<T>] created by the FIDL
  /// compiler for a specific interface.
  Proxy(this.ctrl);

  /// The control plane for this proxy.
  ///
  /// Methods that manipulate the local proxy (as opposed to sending messages
  /// to the remote implementation of [T]) are exposed on this [ctrl] object to
  /// avoid naming conflicts with the methods of [T].
  final ProxyController<T> ctrl;

  // In general it's probably better to avoid adding fields and methods to this
  // class. Names added to this class have to be mangled by bindings generation
  // to avoid name conflicts.
}
