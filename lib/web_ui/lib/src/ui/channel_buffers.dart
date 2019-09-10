// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

/// A saved platform message for a channel with its callback.
class StoredMessage {
  /// Default constructor, takes in a [ByteData] that represents the
  /// payload of the message and a [PlatformMessageResponseCallback]
  /// that represents the callback that will be called when the message
  /// is handled.
  StoredMessage(this._data, this._callback);

  final ByteData _data;
  final PlatformMessageResponseCallback _callback;

  ByteData get data => _data;
  PlatformMessageResponseCallback get callback => _callback;
}

/// A fixed-size circular queue.
class _RingBuffer<T> {
  final ListQueue<T> _queue;
  int _capacity;

  _RingBuffer(this._capacity)
    : _queue = ListQueue<T>(_capacity);

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

/// Storage of channel messages until the channels are completely routed.
///
/// Each channel has a finite buffer capacity and in a FIFO manner messages will
/// be deleted if the capacity is exceeded.  The intention is that these buffers
/// will be drained once a callback is setup on the BinaryMessenger in the
/// Flutter framework.
class ChannelBuffers {
  /// A somewhat arbitrary size that tries to balance handling typical
  /// cases and not wasting memory.
  static const int kDefaultBufferSize = 100;

  final Map<String, _RingBuffer<StoredMessage>> _messages =
    <String, _RingBuffer<StoredMessage>>{};

  /// Returns true on overflow.
  bool push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    _RingBuffer<StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _RingBuffer<StoredMessage>(kDefaultBufferSize);
      _messages[channel] = queue;
    }
    return queue.push(StoredMessage(data, callback));
  }

  /// Returns null on underflow.
  StoredMessage pop(String channel) {
    final _RingBuffer<StoredMessage> queue = _messages[channel];
    final StoredMessage result = queue?.pop();
    return result;
  }

  bool isEmpty(String channel) {
    final _RingBuffer<StoredMessage> queue = _messages[channel];
    return (queue == null) ? true : queue.isEmpty;
  }

  void resize(String channel, int newSize) {
    _RingBuffer<StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = _RingBuffer<StoredMessage>(newSize);
      _messages[channel] = queue;
    } else {
      queue.resize(newSize);
    }
  }
}

final ChannelBuffers channelBuffers = ChannelBuffers();
