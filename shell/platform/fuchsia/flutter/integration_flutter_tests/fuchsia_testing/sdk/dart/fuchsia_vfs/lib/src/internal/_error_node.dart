// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:zircon/zircon.dart';

// ignore_for_file: public_member_api_docs, unnecessary_null_comparison

typedef OnEventSent = Function(ErrorNodeForSendingEvent);

/// This node implementation is used to send on_open event if
/// there is a error opening connection to [Node].
class ErrorNodeForSendingEvent extends Node {
  final int _status;
  final OnEventSent _onEventSent;
  final NodeBinding _bindings = NodeBinding();

  /// Constructor
  ErrorNodeForSendingEvent(
      this._status, this._onEventSent, InterfaceRequest<Node> request)
      : assert(_status != ZX.OK),
        assert(request != null),
        assert(_onEventSent != null) {
    _bindings.bind(this, request);
  }

  @override
  Stream<Node$OnOpen$Response> get onOpen async* {
    yield Node$OnOpen$Response(_status, null);
    _onEventSent(this);
    _bindings.close();
  }

  @override
  Future<void> clone(int flags, InterfaceRequest<Node> object) async {
    // nothing to clone, left blank
  }

  @override
  Future<int> close() async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<NodeInfo> describe() async {
    throw UnsupportedError('ErrorNodeForSendingEvent.describe is unreachable.');
  }

  @override
  Future<Node$GetAttr$Response> getAttr() async {
    throw UnsupportedError('ErrorNodeForSendingEvent.getAttr is unreachable.');
  }

  @override
  Future<int> setAttr(int flags, NodeAttributes attributes) async {
    return ZX.ERR_NOT_SUPPORTED;
  }

  @override
  Future<int> sync() async {
    return ZX.ERR_NOT_SUPPORTED;
  }
}
