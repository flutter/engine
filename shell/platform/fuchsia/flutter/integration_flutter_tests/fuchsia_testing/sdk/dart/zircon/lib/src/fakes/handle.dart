// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon_fakes;

// ignore_for_file: public_member_api_docs

class Handle {
  // No public constructor - this can only be created from native code.
  // ignore: unused_element
  Handle._();

  // Create an invalid handle object.
  Handle.invalid();

  int get handle => -1;

  int get koid => -1;

  @override
  String toString() => 'Handle($handle)';

  @override
  bool operator ==(Object other) =>
      (other is Handle) && (handle == other.handle);

  @override
  int get hashCode => handle.hashCode;

  // Common handle operations.
  bool get isValid => false;
  int close() {
    return 0;
  }

  HandleWaiter asyncWait(int signals, AsyncWaitCallback callback) {
    throw UnimplementedError(
        'Handle.asyncWait() is not implemented on this platform.');
  }

  Handle duplicate(int options) {
    throw UnimplementedError(
        'Handle.duplicate() is not implemented on this platform.');
  }
}
