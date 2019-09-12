// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of ui;

typedef DrainChannelCallback = Future<void> Function(ByteData, PlatformMessageResponseCallback);

/// Remove and process all stored messages for a given channel.
///
/// This should be called once a channel is prepared to handle messages
/// (ie when a message handler is setup in the framework).
void drainChannelBuffer(String channel, DrainChannelCallback callback) async {
  // noop, it may not be possible to store messages in the web_ui.
}
