// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of dart.ui;

/// Signature for [ChannelBuffers.drain]'s `callback` argument.
///
/// The first argument is the data sent by the plugin.
///
/// The second argument is a closure that, when called, will send messages
/// back to the plugin.
// TODO(ianh): deprecate this once the framework is migrated to [ChannelCallback].
typedef DrainChannelCallback = Future<void> Function(ByteData? data, PlatformMessageResponseCallback callback);

/// Signature for [ChannelBuffers.setListener]'s `callback` argument.
///
/// The first argument is the data sent by the plugin.
///
/// The second argument is a closure that, when called, will send messages
/// back to the plugin.
///
/// See also:
///
///  * [PlatformMessageResponseCallback], the type used for replies.
typedef ChannelCallback = void Function(ByteData? data, PlatformMessageResponseCallback callback);

/// The data and logic required to store and invoke a callback.
///
/// This tracks (and applies) the [Zone].
class _ChannelCallbackRecord {
  _ChannelCallbackRecord(this.callback) : zone = Zone.current;
  final ChannelCallback callback;
  final Zone zone;

  /// Call [callback] in [zone], using the given arguments.
  void invoke(ByteData? dataArg, PlatformMessageResponseCallback callbackArg) {
    _invoke2<ByteData?, PlatformMessageResponseCallback>(callback, zone, dataArg, callbackArg);
  }
}

/// A saved platform message for a channel with its callback.
class _StoredMessage {
  /// Wraps the data and callback for a platform message into
  /// a [_StoredMessage] instance.
  ///
  /// The first argument is a [ByteData] that represents the
  /// payload of the message and a [PlatformMessageResponseCallback]
  /// that represents the callback that will be called when the message
  /// is handled.
  const _StoredMessage(this.data, this.callback);

  /// Representation of the message's payload.
  final ByteData? data;

  /// Callback to be used when replying to the message.
  final PlatformMessageResponseCallback callback;
}

/// The internal storage for a platform channel.
///
/// This consists of a fixed-size circular queue of [_StoredMessage]s,
/// and the channel's callback, if any has been registered.
class _Channel {
  _Channel(this._capacity)
    : _queue = collection.ListQueue<_StoredMessage>(_capacity);

  /// The underlying data for the buffered messages.
  final collection.ListQueue<_StoredMessage> _queue;

  /// The number of messages currently in the [_Channel].
  ///
  /// This is equal to or less than the [capacity].
  int get length => _queue.length;

  /// The number of messages that _can_ be stored in the [_Channel].
  ///
  /// When additional messages are stored, earlier ones are discarded,
  /// in a first-in-first-out fashion.
  int get capacity => _capacity;
  int _capacity;
  /// Set the [capacity] of the channel to the given size.
  ///
  /// If the new size is smaller than the [length], the oldest
  /// messages are discarded until the capacity is reached.
  set capacity(int newSize) {
    _capacity = newSize;
    _dropOverflowMessages(newSize);
  }

  /// Whether a microtask is queued to call [_drainStep].
  ///
  /// This is used to queue messages received while draining, rather
  /// than sending them out of order. This generally cannot happen in
  /// production but is possible in test scenarios.
  ///
  /// This is also necessary to avoid situations where multiple drains are
  /// invoked simultaneously. For example, if a listener is set
  /// (queuing a drain), then unset, then set again (which would queue
  /// a drain again), all in one stack frame (not allowing the drain
  /// itself an opportunity to check if a listener is set).
  bool _draining = false;

  /// Adds a message to the channel.
  ///
  /// Returns true on overflow. Earlier messages are discarded,
  /// in a first-in-first-out fashion. See [capacity].
  bool push(_StoredMessage message) {
    if (!_draining && _channelCallbackRecord != null) {
      assert(_queue.isEmpty);
      _channelCallbackRecord!.invoke(message.data, message.callback);
      return false;
    }
    if (_capacity <= 0) {
      return true;
    }
    final int overflowCount = _dropOverflowMessages(_capacity - 1);
    _queue.addLast(message);
    return overflowCount > 0;
  }

  /// Returns the first message in the channel and removes it.
  ///
  /// Throws when empty.
  _StoredMessage pop() => _queue.removeFirst();

  /// Removes messages until [length] reaches `lengthLimit`, and returns
  /// the number of messages removed.
  int _dropOverflowMessages(int lengthLimit) {
    int result = 0;
    while (_queue.length > lengthLimit) {
      final _StoredMessage message = _queue.removeFirst();
      message.callback(null); // send empty reply to the plugin side
      result += 1;
    }
    return result;
  }

  _ChannelCallbackRecord? _channelCallbackRecord;

  /// Sets the listener for this channel.
  ///
  /// When there is a listener, messages are sent immediately.
  ///
  /// If any messages were queued before the listener is added,
  /// they are drained asynchronously after this method returns.
  /// (See [_drain].)
  ///
  /// Only one listener may be set at a time. Setting a
  /// new listener clears the previous one.
  ///
  /// Callbacks are invoked in their own stack frame and
  /// use the zone that was current when the callback was
  /// registered.
  void setListener(ChannelCallback callback) {
    final bool needDrain = _channelCallbackRecord == null;
    _channelCallbackRecord = _ChannelCallbackRecord(callback);
    if (needDrain && !_draining)
      _drain();
  }

  /// Clears the listener for this channel.
  ///
  /// When there is no listener, messages are queued, up to [capacity],
  /// and then discarded in a first-in-first-out fashion.
  void clearListener() {
    _channelCallbackRecord = null;
  }

  /// Drains all the messages in the channel (invoking the currently
  /// registered listener for each one).
  ///
  /// Each message is handled in its own microtask. No messages can
  /// be queued by plugins while the queue is being drained, but any
  /// microtasks queued by the handler itself will be processed before
  /// the next message is handled.
  ///
  /// The draining stops if the listener is removed.
  ///
  /// See also:
  ///
  ///  * [setListener], which is used to register the callback.
  ///  * [clearListener], which removes it.
  void _drain() {
    assert(!_draining);
    _draining = true;
    scheduleMicrotask(_drainStep);
  }

  /// Drains a single message and then reinvokes itself asynchronously.
  ///
  /// See [_drain] for more details.
  void _drainStep() {
    assert(_draining);
    if (_queue.isNotEmpty && _channelCallbackRecord != null) {
      final _StoredMessage message = pop();
      _channelCallbackRecord!.invoke(message.data, message.callback);
      scheduleMicrotask(_drainStep);
    } else {
      _draining = false;
    }
  }
}

/// The buffering and dispatch mechanism for messages sent by plugins
/// on the engine side to their corresponding plugin code on the
/// framework side.
///
/// Messages for a channel are stored until a listener is provided for that channel,
/// using [setListener]. Only one listener may be configured per channel.
///
/// Each channel has a finite buffer capacity and messages will
/// be deleted in a first-in-first-out (FIFO) manner if the capacity is exceeded.
///
/// Typically these buffers are drained once a callback is setup on
/// the [BinaryMessenger] in the Flutter framework. (See [setListener].)
class ChannelBuffers {
  /// Create a buffer pool for platform messages.
  ///
  /// It is generally not necessary to create an instance of this class;
  /// the global [channelBuffers] instance is the one used by the engine.
  ChannelBuffers();

  /// The number of messages that channel buffers will store by default.
  ///
  /// By default buffers store one message per channel.
  ///
  /// There are tradeoffs associated with any size. The correct size
  /// should be chosen for the semantics of the channel. To change the
  /// size a plugin can send a message using the control channel,
  /// whose name is given by [kControlChannelName].
  ///
  /// Size 0 is appropriate for channels where channels sent before
  /// the engine and framework are ready should be ignored. For
  /// example, a plugin that notifies the framework any time a
  /// radiation sensor detects an ionization event might set its size
  /// to zero since past ionization events are typically not
  /// interesting, only instantaneous readings are worth tracking.
  ///
  /// Size 1 is appropriate for level-triggered plugins. For example,
  /// a plugin that notifies the framework of the current value of a
  /// pressure sensor might leave its size at one (the default), while
  /// sending messages continually; once the framework side of the plugin
  /// registers with the channel, it will immediately receive the most
  /// up to date value and earlier messages will have been discarded.
  ///
  /// Sizes greater than one are appropriate for plugins where every
  /// message is important. For example, a plugin that itself
  /// registers with another system that has been buffering events,
  /// and immediately forwards all the previously-buffered events,
  /// would likely wish to avoid having any messages dropped on the
  /// floor. In such situations, it is important to select a size that
  /// will avoid overflows. It is also important to consider the
  /// potential for the framework side to never fully initialize (e.g. if
  /// the user starts the application, but terminates it soon
  /// afterwards, leaving time for the platform side of a plugin to
  /// run but not the framework side).
  static const int kDefaultBufferSize = 1;

  /// The name of the channel that plugins can use to communicate with the
  /// channel buffers system.
  ///
  /// These messages are handled by [handleMessage].
  static const String kControlChannelName = 'dev.flutter/channel-buffers';

  /// A mapping between a channel name and its associated [_Channel].
  final Map<String, _Channel> _channels = <String, _Channel>{};

  /// Adds a message (`data`) to the named channel buffer (`name`).
  ///
  /// The `callback` argument is a closure that, when called, will send messages
  /// back to the plugin.
  ///
  /// Returns true on overflow.
  bool push(String name, ByteData? data, PlatformMessageResponseCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel(kDefaultBufferSize));
    return channel.push(_StoredMessage(data, callback));
  }

  /// Sets the listener for the specified channel.
  ///
  /// When there is a listener, messages are sent immediately.
  ///
  /// Each channel may have up to one listener set at a time. Setting
  /// a new listener on a channel with an existing listener clears the
  /// previous one.
  ///
  /// Callbacks are invoked in their own stack frame and
  /// use the zone that was current when the callback was
  /// registered.
  ///
  /// ## Draining
  ///
  /// If any messages were queued before the listener is added,
  /// they are drained asynchronously after this method returns.
  ///
  /// Each message is handled in its own microtask. No messages can
  /// be queued by plugins while the queue is being drained, but any
  /// microtasks queued by the handler itself will be processed before
  /// the next message is handled.
  ///
  /// The draining stops if the listener is removed.
  void setListener(String name, ChannelCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel(kDefaultBufferSize));
    channel.setListener(callback);
  }

  /// Clears the listener for the specified channel.
  ///
  /// When there is no listener, messages on that channel are queued,
  /// up to [kDefaultBufferSize] (or the size configured via the
  /// control channel), and then discarded in a first-in-first-out
  /// fashion.
  void clearListener(String name) {
    final _Channel? channel = _channels[name];
    if (channel != null)
      channel.clearListener();
  }

  /// Remove and process all stored messages for a given channel.
  ///
  /// This should be called once a channel is prepared to handle messages
  /// (i.e. when a message handler is setup in the framework).
  ///
  /// The messages are processed by calling the given `callback`. Each message
  /// is processed in its own microtask.
  // TODO(ianh): deprecate once framework uses [setListener].
  Future<void> drain(String name, DrainChannelCallback callback) async {
    final _Channel? channel = _channels[name];
    while (channel != null && !channel._queue.isEmpty) {
      final _StoredMessage message = channel.pop();
      await callback(message.data, message.callback);
    }
  }

  String _getString(ByteData data) {
    final ByteBuffer buffer = data.buffer;
    final Uint8List list = buffer.asUint8List(data.offsetInBytes, data.lengthInBytes);
    return utf8.decode(list);
  }

  /// Handle a control message.
  ///
  /// This is intended to be called by the platform messages dispatcher.
  ///
  /// Available messages are listed below.
  ///
  /// ## `resize`
  ///
  /// **Name:** `resize`
  ///
  /// **Arity:** 2 parameters
  ///
  /// **Format:** `resize\r<channel name>\r<new size>`
  ///
  /// **Description:** Allows you to set the size of a channel's buffer.
  void handleMessage(ByteData data) {
    final List<String> message = _getString(data).split('\r');
    if (message.length == 1 + /*arity=*/2 && message[0] == 'resize') {
      _resize(message[1], int.parse(message[2]));
    } else {
      // If the message couldn't be decoded as UTF-8, a FormatException will
      // have been thrown by _getString.
      throw Exception('Unrecognized message $message sent to $kControlChannelName.');
    }
  }

  /// Changes the capacity of the queue associated with the given channel.
  ///
  /// This could result in the dropping of messages if newSize is less
  /// than the current length of the queue.
  void _resize(String name, int newSize) {
    _Channel? channel = _channels[name];
    if (channel == null) {
      channel = _Channel(newSize);
      _channels[name] = channel;
    } else {
      channel.capacity = newSize;
    }
  }
}

/// [ChannelBuffers] that allow the storage of messages between the
/// Engine and the Framework.  Typically messages that can't be delivered
/// are stored here until the Framework is able to process them.
///
/// See also:
///
/// * [BinaryMessenger], where [ChannelBuffers] are typically read.
final ChannelBuffers channelBuffers = ChannelBuffers();
