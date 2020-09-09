// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of engine;

class _ChannelCallbackRecord {
  _ChannelCallbackRecord(this.callback) : zone = Zone.current;
  final ui.ChannelCallback callback;
  final Zone zone;

  void invoke(ByteData? dataArg, ui.PlatformMessageResponseCallback callbackArg) {
    _invoke2<ByteData?, ui.PlatformMessageResponseCallback>(callback, zone, dataArg, callbackArg);
  }
}

class _StoredMessage {
  const _StoredMessage(this.data, this.callback);

  final ByteData? data;

  final ui.PlatformMessageResponseCallback callback;
}

class _Channel {
  _Channel(this._capacity)
    : _queue = ListQueue<_StoredMessage>(_capacity);

  final ListQueue<_StoredMessage> _queue;

  int get length => _queue.length;

  int _capacity;
  int get capacity => _capacity;

  int resize(int newSize) {
    _capacity = newSize;
    return _dropOverflowMessages(newSize);
  }

  bool _draining = false;

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

  _StoredMessage pop() => _queue.removeFirst();

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

  void setListener(ui.ChannelCallback callback) {
    final bool needDrain = _channelCallbackRecord == null;
    _channelCallbackRecord = _ChannelCallbackRecord(callback);
    if (needDrain && !_draining)
      _drain();
  }

  void clearListener() {
    _channelCallbackRecord = null;
  }

  void _drain() {
    assert(!_draining);
    _draining = true;
    scheduleMicrotask(_drainStep);
  }

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

class EngineChannelBuffers extends ui.ChannelBuffers {
  final Map<String, _Channel> _channels = <String, _Channel>{};

  @override
  bool push(String name, ByteData? data, ui.PlatformMessageResponseCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel(ui.ChannelBuffers.kDefaultBufferSize));
    return channel.push(_StoredMessage(data, callback));
  }

  @override
  void setListener(String name, ui.ChannelCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel(ui.ChannelBuffers.kDefaultBufferSize));
    channel.setListener(callback);
  }

  @override
  void clearListener(String name) {
    final _Channel? channel = _channels[name];
    if (channel != null)
      channel.clearListener();
  }

  @override
  Future<void> drain(String name, ui.DrainChannelCallback callback) async {
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

  @override
  void handleMessage(ByteData data) {
    final List<String> command = _getString(data).split('\r');
    if (command.length == 1 + /*arity=*/2 && command[0] == 'resize') {
      _resize(command[1], int.parse(command[2]));
    } else {
      throw Exception('Unrecognized command $command sent to ${ui.ChannelBuffers.kControlChannelName}.');
    }
  }

  void _resize(String name, int newSize) {
    _Channel? channel = _channels[name];
    if (channel == null) {
      channel = _Channel(newSize);
      _channels[name] = channel;
    } else {
      channel.resize(newSize);
    }
  }
}

final ui.ChannelBuffers channelBuffers = EngineChannelBuffers();
