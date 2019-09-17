// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

/// A saved platform message for a channel with its callback.
class _StoredMessage {
  /// Default constructor, takes in a [ByteData] that represents the
  /// payload of the message and a [PlatformMessageResponseCallback]
  /// that represents the callback that will be called when the message
  /// is handled.
  _StoredMessage(this._data, this._callback);

  final ByteData _data;
  final PlatformMessageResponseCallback _callback;

  /// Getter for _data field, represents the message's payload.
  ByteData get data => _data;
  /// Getter for the _callback field, to be called when the message is received.
  PlatformMessageResponseCallback get callback => _callback;
}

/// A fixed-size circular queue.
class _RingBuffer<T> {
  final collection.ListQueue<T> _queue;

  _RingBuffer(this._capacity)
    : _queue = collection.ListQueue<T>(_capacity);

  /// Returns the number of items in the [_RingBuffer].
  int get length => _queue.length;

  /// The number of items that can be stored in the [_RingBuffer].
  int _capacity;
  int get capacity => _capacity;

  /// Returns true if there are no items in the [_RingBuffer].
  bool get isEmpty => _queue.isEmpty;

  /// A callback that get's called when items are ejected from the [_RingBuffer]
  /// by way of an overflow or a resizing.
  Function(T) _dropItemCallback;
  set dropItemCallback(Function(T) callback) {
    _dropItemCallback = callback;
  }

  /// Returns true on overflow.
  bool push(T val) {
    if (_capacity <= 0) {
      return true;
    } else {
      final int overflowCount = _dropOverflowItems(_capacity - 1);
      _queue.addLast(val);
      return overflowCount > 0;
    }
  }

  /// Returns null when empty.
  T pop() {
    return _queue.isEmpty ? null : _queue.removeFirst();
  }

  /// Removes items until then length reaches [lengthLimit] and returns
  /// the number of items removed.
  int _dropOverflowItems(int lengthLimit) {
    int result = 0;
    while (_queue.length > lengthLimit) {
      final T item = _queue.removeFirst();
      if (_dropItemCallback != null) {
        _dropItemCallback(item);
      }
      result += 1;
    }
    return result;
  }

  /// Returns the number of discarded items resulting from resize.
  int resize(int newSize) {
    _capacity = newSize;
    return _dropOverflowItems(newSize);
  }
}

/// A callback for [ChannelBuffers.drain], called as it pops stored messages.
typedef DrainChannelCallback = Future<void> Function(ByteData, PlatformMessageResponseCallback);

/// Storage of channel messages until the channels are completely routed,
/// i.e. when a message handler is attached to the channel on the framework side.
///
/// Each channel has a finite buffer capacity and in a FIFO manner messages will
/// be deleted if the capacity is exceeded.  The intention is that these buffers
/// will be drained once a callback is setup on the BinaryMessenger in the
/// Flutter framework.
///
/// Clients of Flutter shouldn't need to allocate their own ChannelBuffers
/// and should only access this package's [channelBuffers] if they are writing
/// their own custom [BinaryMessenger].
class ChannelBuffers {
  /// By default we store one message per channel.  There are tradeoffs associated
  /// with any size.  The correct size should be chosen for the semantics of your
  /// channel.
  ///
  /// Size 0 implies you want to ignore any message that gets sent before the engine
  /// is ready (keeping in mind there is no way to know when the engine is ready).
  ///
  /// Size 1 implies that you only care about the most recent value.
  ///
  /// Size >1 means you want to process every single message and want to chose a
  /// buffer size that will avoid any overflows.
  static const int kDefaultBufferSize = 1;

  final Map<String, _RingBuffer<_StoredMessage>> _messages =
    <String, _RingBuffer<_StoredMessage>>{};

  _RingBuffer<_StoredMessage> _makeRingBuffer(int size) {
    final _RingBuffer<_StoredMessage> result = _RingBuffer<_StoredMessage>(size);
    result.dropItemCallback = _onDropItem;
    return result;
  }

  void _onDropItem(_StoredMessage message) {
    message.callback(null);
  }

  /// Returns true on overflow.
  bool push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    _RingBuffer<_StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _makeRingBuffer(kDefaultBufferSize);
      _messages[channel] = queue;
    }
    final bool result = queue.push(_StoredMessage(data, callback));
    if (result) {
      // TODO(aaclarke): Update this message to include instructions on how to resize
      // the buffer once that is available to users.
      _Logger._printString('Overflow on channel: $channel.  '
                           'Messages on this channel are being discarded.  '
                           'The engine may not be running or you need to adjust '
                           'the buffer size if of the channel.');
    }
    return result;
  }

  /// Returns null on underflow.
  _StoredMessage _pop(String channel) {
    final _RingBuffer<_StoredMessage> queue = _messages[channel];
    final _StoredMessage result = queue?.pop();
    return result;
  }

  bool _isEmpty(String channel) {
    final _RingBuffer<_StoredMessage> queue = _messages[channel];
    return (queue == null) ? true : queue.isEmpty;
  }

  /// Changes the capacity of the queue associated with the given channel.
  ///
  /// This could result in the dropping of messages if the newSize is less
  /// than the current length of the queue.
  void resize(String channel, int newSize) {
    _RingBuffer<_StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _makeRingBuffer(newSize);
      _messages[channel] = queue;
    } else {
      final int numberOfDroppedMessages = queue.resize(newSize);
      if (numberOfDroppedMessages > 0) {
        _Logger._printString('Dropping messages on channel "$channel" as a result of shrinking the buffer size.');
      }
    }
  }

  /// Remove and process all stored messages for a given channel.
  ///
  /// This should be called once a channel is prepared to handle messages
  /// (ie when a message handler is setup in the framework).
  Future<void> drain(String channel, DrainChannelCallback callback) async {
    while (!_isEmpty(channel)) {
      final _StoredMessage message = _pop(channel);
      await callback(message.data, message.callback);
    }
  }
}

final ChannelBuffers channelBuffers = ChannelBuffers();
