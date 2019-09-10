// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.ui;

class StoredMessage {
  final ByteData _data;
  final PlatformMessageResponseCallback _callback;

  StoredMessage(this._data, this._callback);

  ByteData get data => _data;

  PlatformMessageResponseCallback get callback => _callback;
}

/// A fixed-size circular queue.
class RingBuffer<T> {
  final collection.ListQueue<T> _queue;
  final int _capacity;

  RingBuffer(this._capacity) 
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

  T pop() {
    return _queue.isEmpty ? null : _queue.removeFirst();
  }
}

/// Storage of channel messages until the channels are completely routed.
class ChannelBuffers {
  static const int DEFAULT_BUFFER_SIZE = 100;

  final Map<String, RingBuffer<StoredMessage>> _messages = {};

  void push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    RingBuffer<StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = RingBuffer<StoredMessage>(DEFAULT_BUFFER_SIZE);
      _messages[channel] = queue;
    }
    if (queue.push(StoredMessage(data, callback))) {
      _Logger._printString('Overflow on channel:' + channel);
    }
  }

  StoredMessage pop(String channel) {
    final RingBuffer<StoredMessage> queue = _messages[channel];
    final StoredMessage result = queue?.pop();
    if (result == null) {
      _Logger._printString('Underflow on channel:' + channel);
    }
    return result;
  }

  bool isEmpty(String channel) {
    final RingBuffer<StoredMessage> queue = _messages[channel];
    return queue?.isEmpty ?? true;
  }

  void resize(String channel, int newSize) {
  }
}

final ChannelBuffers channelBuffers = ChannelBuffers();
