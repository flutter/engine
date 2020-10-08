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
  _Channel([ this._capacity = ui.ChannelBuffers.kDefaultBufferSize ])
    : _queue = ListQueue<_StoredMessage>(_capacity);

  final ListQueue<_StoredMessage> _queue;

  int get length => _queue.length;
  bool debugEnableDiscardWarnings = true;

  int get capacity => _capacity;
  int _capacity;
  set capacity(int newSize) {
    _capacity = newSize;
    _dropOverflowMessages(newSize);
  }

  bool _draining = false;

  bool push(_StoredMessage message) {
    if (!_draining && _channelCallbackRecord != null) {
      assert(_queue.isEmpty);
      _channelCallbackRecord!.invoke(message.data, message.callback);
      return false;
    }
    if (_capacity <= 0) {
      return debugEnableDiscardWarnings;
    }
    final bool result = _dropOverflowMessages(_capacity - 1);
    _queue.addLast(message);
    return result;
  }

  _StoredMessage pop() => _queue.removeFirst();

  bool _dropOverflowMessages(int lengthLimit) {
    bool result = false;
    while (_queue.length > lengthLimit) {
      final _StoredMessage message = _queue.removeFirst();
      message.callback(null); // send empty reply to the plugin side
      result = true;
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
  void push(String name, ByteData? data, ui.PlatformMessageResponseCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel());
    if (channel.push(_StoredMessage(data, callback))) {
      assert(() {
        print(
          'A message on the $name channel was discarded before it could be handled.\n'
          'This happens when a plugin sends messages to the framework side before the '
          'framework has had an opportunity to register a listener. See the ChannelBuffers '
          'API documentation for details on how to configure the channel to expect more '
          'messages, or to expect messages to get discarded:\n'
          '  https://api.flutter.dev/flutter/dart-ui/ChannelBuffers-class.html'
        );
        return true;
      }());
    }
  }

  @override
  void setListener(String name, ui.ChannelCallback callback) {
    final _Channel channel = _channels.putIfAbsent(name, () => _Channel());
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

  void resize(String name, int newSize) {
    _Channel? channel = _channels[name];
    if (channel == null) {
      channel = _Channel(newSize);
      _channels[name] = channel;
    } else {
      channel.capacity = newSize;
    }
  }

  void allowOverflow(String name, bool allowed) {
    assert(() {
      _Channel? channel = _channels[name];
      if (channel == null && allowed) {
        channel = _Channel();
        _channels[name] = channel;
      }
      channel?.debugEnableDiscardWarnings = !allowed;
      return true;
    }());
  }
}

final ui.ChannelBuffers channelBuffers = EngineChannelBuffers();
