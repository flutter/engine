// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

typedef DrainChannelCallback = Future<void> Function(ByteData, PlatformMessageResponseCallback);

/// Storage of channel messages until the channels are completely routed
/// (ie when a message handler is attached to the channel on the framework side).
///
/// Each channel has a finite buffer capacity and in a FIFO manner messages will
/// be deleted if the capacity is exceeded.  The intention is that these buffers
/// will be drained once a callback is setup on the BinaryMessenger in the
/// Flutter framework.
class ChannelBuffers {
  /// Returns true on overflow.
  bool push(String channel, ByteData data, PlatformMessageResponseCallback callback) {
    return true;
  }

  void resize(String channel, int newSize) {
  }

  /// Remove and process all stored messages for a given channel.
  ///
  /// This should be called once a channel is prepared to handle messages
  /// (ie when a message handler is setup in the framework).
  Future<void> drain(String channel, DrainChannelCallback callback) async {
  }
}

final ChannelBuffers channelBuffers = ChannelBuffers();
