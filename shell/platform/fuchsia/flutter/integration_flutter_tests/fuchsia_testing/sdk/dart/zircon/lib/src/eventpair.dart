// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

// ignore_for_file: public_member_api_docs
// ignore_for_file: constant_identifier_names

/// Typed wrapper around a Zircon eventpair object.
class EventPair extends _HandleWrapper<EventPair> {
  EventPair(Handle? handle) : super(handle);

  /// Duplicate this [EventPair] with the given rights.
  EventPair duplicate(int rights) {
    return EventPair(handle!.duplicate(rights));
  }

  // Signals
  static const int SIGNALED = ZX.EVENTPAIR_SIGNALED;
  static const int PEER_CLOSED = ZX.EVENTPAIR_PEER_CLOSED;
}

/// Typed wrapper around a linked pair of eventpair objects and the
/// zx_eventpair_create() syscall used to create them.
class EventPairPair extends _HandleWrapperPair<EventPair?> {
  factory EventPairPair() {
    final HandlePairResult result = System.eventpairCreate();
    if (result.status == ZX.OK) {
      return EventPairPair._(
          result.status, EventPair(result.first), EventPair(result.second));
    } else {
      return EventPairPair._(result.status, null, null);
    }
  }

  EventPairPair._(int status, EventPair? first, EventPair? second)
      : super._(status, first, second);
}
