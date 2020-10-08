// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of ui;

typedef DrainChannelCallback = Future<void> Function(ByteData? data, PlatformMessageResponseCallback callback);

typedef ChannelCallback = void Function(ByteData? data, PlatformMessageResponseCallback callback);

abstract class ChannelBuffers {
  ChannelBuffers();
  static const int kDefaultBufferSize = 1;
  static const String kControlChannelName = 'dev.flutter/channel-buffers';
  void push(String name, ByteData? data, PlatformMessageResponseCallback callback);
  void setListener(String name, ChannelCallback callback);
  void clearListener(String name);
  Future<void> drain(String name, DrainChannelCallback callback);
  void handleMessage(ByteData data) {
    final Uint8List bytes = data.buffer.asUint8List(data.offsetInBytes, data.lengthInBytes);
    if (bytes[0] == 0x07) {
      int methodNameLength = bytes[1];
      if (methodNameLength >= 254)
        throw Exception('Unrecognized message sent to $kControlChannelName (method name too long)');
      int index = 2;
      String methodName = utf8.decode(bytes.sublist(index, index + methodNameLength));
      index += methodNameLength;
      switch (methodName) {
        case 'resize':
          if (bytes[index] != 0x0C)
            throw Exception('Invalid arguments for \'resize\' method sent to $kControlChannelName (arguments must be a two-element list, channel name and new capacity)');
          index += 1;
          if (bytes[index] < 0x02)
            throw Exception('Invalid arguments for \'resize\' method sent to $kControlChannelName (arguments must be a two-element list, channel name and new capacity)');
          index += 1;
          if (bytes[index] != 0x07)
            throw Exception('Invalid arguments for \'resize\' method sent to $kControlChannelName (first argument must be a string)');
          index += 1;
          int channelNameLength = bytes[index];
          if (channelNameLength >= 254)
            throw Exception('Invalid arguments for \'resize\' method sent to $kControlChannelName (channel name must be less than 254 characters long)');
          index += 1;
          String channelName = utf8.decode(bytes.sublist(index, index + channelNameLength));
          index += channelNameLength;
          if (bytes[index] != 0x03)
            throw Exception('Invalid arguments for \'resize\' method sent to $kControlChannelName (second argument must be an integer in the range 0 to 2147483647)');
          index += 1;
          resize(channelName, data.getUint32(index, Endian.host));
          break;
        case 'overflow':
          if (bytes[index] != 0x0C)
            throw Exception('Invalid arguments for \'overflow\' method sent to $kControlChannelName (arguments must be a two-element list, channel name and flag state)');
          index += 1;
          if (bytes[index] < 0x02)
            throw Exception('Invalid arguments for \'overflow\' method sent to $kControlChannelName (arguments must be a two-element list, channel name and flag state)');
          index += 1;
          if (bytes[index] != 0x07)
            throw Exception('Invalid arguments for \'overflow\' method sent to $kControlChannelName (first argument must be a string)');
          index += 1;
          int channelNameLength = bytes[index];
          if (channelNameLength >= 254)
            throw Exception('Invalid arguments for \'overflow\' method sent to $kControlChannelName (channel name must be less than 254 characters long)');
          index += 1;
          String channelName = utf8.decode(bytes.sublist(index, index + channelNameLength));
          index += channelNameLength;
          if (bytes[index] != 0x01 && bytes[index] != 0x02)
            throw Exception('Invalid arguments for \'overflow\' method sent to $kControlChannelName (second argument must be a boolean)');
          allowOverflow(channelName, bytes[index] == 0x01);
          break;
        default:
          throw Exception('Unrecognized method \'$methodName\' sent to $kControlChannelName');
      }
    } else {
      final List<String> parts = utf8.decode(bytes).split('\r');
      if (parts.length == 1 + 2 && parts[0] == 'resize') {
        resize(parts[1], int.parse(parts[2]));
      } else {
        throw Exception('Unrecognized message $parts sent to $kControlChannelName.');
      }
    }
  }
  void resize(String name, int newSize);
  void allowOverflow(String name, bool allowed);
}

ChannelBuffers get channelBuffers => engine.channelBuffers;
