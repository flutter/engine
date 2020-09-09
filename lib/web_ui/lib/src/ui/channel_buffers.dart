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
  bool push(String name, ByteData? data, PlatformMessageResponseCallback callback);
  void setListener(String name, ChannelCallback callback);
  void clearListener(String name);
  Future<void> drain(String name, DrainChannelCallback callback);
  void handleMessage(ByteData data);
}

ChannelBuffers get channelBuffers => engine.channelBuffers;
