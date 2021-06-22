// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

// ignore_for_file: constant_identifier_names
// ignore_for_file: public_member_api_docs

/// Typed wrapper around a Zircon channel object.
class Channel extends _HandleWrapper<Channel> {
  Channel(Handle? handle) : super(handle);

  /// Gets a Channel for accessing the file at `path`.
  ///
  /// The returned Channel supports read-only fdio operations on the file.
  factory Channel.fromFile(String path) {
    HandleResult r = System.channelFromFile(path);
    if (r.status != ZX.OK) {
      throw ZxStatusException(r.status, getStringForStatus(r.status));
    }
    return Channel(r.handle);
  }

  // Signals
  static const int READABLE = ZX.CHANNEL_READABLE;
  static const int WRITABLE = ZX.CHANNEL_WRITABLE;
  static const int PEER_CLOSED = ZX.CHANNEL_PEER_CLOSED;

  // Read options
  static const int READ_MAY_DISCARD = ZX.CHANNEL_READ_MAY_DISCARD;

  // Limits
  static const int MAX_MSG_BYTES = ZX.CHANNEL_MAX_MSG_BYTES;
  static const int MAX_MSG_HANDLES = ZX.CHANNEL_MAX_MSG_HANDLES;

  int write(ByteData data, [List<Handle>? handles]) {
    if (handle == null) {
      return ZX.ERR_INVALID_ARGS;
    }

    return System.channelWrite(handle!, data, handles ?? []);
  }

  ReadResult queryAndRead() {
    if (handle == null) {
      return const ReadResult(ZX.ERR_INVALID_ARGS);
    }
    return System.channelQueryAndRead(handle!);
  }
}

/// Typed wrapper around a linked pair of channel objects and the
/// zx_channel_create() syscall used to create them.
class ChannelPair extends _HandleWrapperPair<Channel?> {
  factory ChannelPair() {
    final HandlePairResult result = System.channelCreate();
    if (result.status == ZX.OK) {
      return ChannelPair._(
          result.status, Channel(result.first), Channel(result.second));
    } else {
      return ChannelPair._(result.status, null, null);
    }
  }

  ChannelPair._(int status, Channel? first, Channel? second)
      : super._(status, first, second);
}
