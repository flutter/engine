// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon_fakes;

// ignore_for_file: public_member_api_docs, avoid_unused_constructor_parameters

class HandleDisposition {
  HandleDisposition(int operation, Handle handle, int type, int rights);

  int get operation => -1;
  Handle get handle => Handle.invalid();
  int get type => -1;
  int get rights => -1;
  int get result => -1;

  // @pragma('vm:entry-point')
  // const HandleDisposition(this.operation, this.handle, this.type, this.rights, this.result);

  @override
  String toString() =>
      'HandleDisposition(operation=$operation, handle=$handle, type=$type, rights=$rights, result=$result)';
}
