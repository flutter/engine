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
    return _queue.removeFirst();
  }
}

/// Storage of channel messages until the channels are completely routed.
class ChannelBuffers {
  static const int DEFAULT_BUFFER_SIZE = 100;

  // TODO(engine): Convert queue to a ring buffer.
  final Map<String, collection.Queue<StoredMessage>> _messages = {};

  void push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    collection.Queue<StoredMessage> queue = _messages[channel];
    if (queue == null) {
      queue = collection.Queue<StoredMessage>();
      _messages[channel] = queue;
    }
    queue.addLast(StoredMessage(data, callback));
  }

  StoredMessage pop(String channel) {
    final collection.Queue<StoredMessage> queue = _messages[channel];
    return queue?.removeFirst();
  }

  bool isEmpty(String channel) {
    final collection.Queue<StoredMessage> queue = _messages[channel];
    return queue?.isEmpty ?? true;
  }

  void resize(String channel, int newSize) {
  }
}

final ChannelBuffers channelBuffers = ChannelBuffers();
