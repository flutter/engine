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
  int _capacity;

  _RingBuffer(this._capacity)
    : _queue = collection.ListQueue<T>(_capacity);

  int get length => _queue.length;

  int get capacity => _capacity;

  bool get isEmpty => _queue.isEmpty;

  /// Returns true on overflow.
  bool push(T val) {
    bool overflow = false;
    while (_queue.length >= _capacity) {
      _queue.removeFirst();
      overflow = true;
    }
    _queue.addLast(val);
    return overflow;
  }

  /// Returns null when empty.
  T pop() {
    return _queue.isEmpty ? null : _queue.removeFirst();
  }

  /// Returns the number of discarded items resulting from resize.
  int resize(int newSize) {
    int result = 0;

    while (length > newSize) {
      result += 1;
      _queue.removeFirst();
    }

    _capacity = newSize;

    return result;
  }
}

/// Storage of channel messages until the channels are completely routed
/// (ie when a message handler is attached to the channel on the framework side).
///
/// Each channel has a finite buffer capacity and in a FIFO manner messages will
/// be deleted if the capacity is exceeded.  The intention is that these buffers
/// will be drained once a callback is setup on the BinaryMessenger in the
/// Flutter framework.
class ChannelBuffers {
  /// A somewhat arbitrary size that tries to balance handling typical
  /// cases and not wasting memory.
  static const int kDefaultBufferSize = 100;

  final Map<String, _RingBuffer<_StoredMessage>> _messages =
    <String, _RingBuffer<_StoredMessage>>{};

  /// Returns true on overflow.
  bool push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    _RingBuffer<_StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _RingBuffer<_StoredMessage>(kDefaultBufferSize);
      _messages[channel] = queue;
    }
    final bool result = queue.push(_StoredMessage(data, callback));
    if (result) {
      _Logger._printString('Overflow on channel: $channel.  '
                           'Messages on this channel are being sent faster '
                           'than they are being processed which is resulting '
                           'in the dropping of messages.  The engine may not be '
                           'running or you need to adjust the buffer size.');
    }
    return result;
  }

  /// Returns null on underflow.
  _StoredMessage pop(String channel) {
    final _RingBuffer<_StoredMessage> queue = _messages[channel];
    final _StoredMessage result = queue?.pop();
    return result;
  }

  bool isEmpty(String channel) {
    final _RingBuffer<_StoredMessage> queue = _messages[channel];
    return (queue == null) ? true : queue.isEmpty;
  }

  void resize(String channel, int newSize) {
    _RingBuffer<_StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _RingBuffer<_StoredMessage>(newSize);
      _messages[channel] = queue;
    } else {
      final int numberOfDroppedMessages = queue.resize(newSize);
      if (numberOfDroppedMessages > 0) {
        _Logger._printString('Dropping messages on channel "$channel" as a result of shrinking the buffer size.');
      }
    }
  }
}

typedef DrainChannelCallback = Future<void> Function(ByteData, PlatformMessageResponseCallback);

/// Remove and process all stored messages for a given channel.
///
/// This should be called once a channel is prepared to handle messages
/// (ie when a message handler is setup in the framework).
void drainChannelBuffer(String channel, DrainChannelCallback callback) async {
  while (!_channelBuffers.isEmpty(channel)) {
    final _StoredMessage message = _channelBuffers.pop(channel);
    await callback(message.data, message.callback);
  }
}

final ChannelBuffers _channelBuffers = ChannelBuffers();
