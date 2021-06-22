// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl/fidl.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';
import 'package:zircon/zircon.dart';

import 'internal/_error_node.dart';
import 'internal/_flags.dart';

/// This interface declares a abstract Vnode class with
/// common operations that may be overwritten.
///
/// These nodes can be added to [PseudoDir] and can be accessed from filesystem.
abstract class Vnode {
  final List<ErrorNodeForSendingEvent> _errorNodes = [];

  /// Close this node and all of its bindings and children.
  void close();

  /// Connect to this vnode.
  /// All flags and modes are defined in
  /// https://fuchsia.googlesource.com/fuchsia/+/HEAD/zircon/system/fidl/fuchsia-io/io.fidl
  ///
  /// By default param [#parentFlags] is all rights, so that open will allow
  /// all rights requested on the incoming [request].
  /// This param is used by clone to restrict cloning.
  int connect(int flags, int mode, InterfaceRequest<Node> request,
      [int parentFlags = Flags.fsRights]);

  /// Filter flags when [openFlagNodeReference] is passed.
  /// This will maintain compatibility with c++ layer.
  int filterForNodeReference(int flags) {
    if (Flags.isNodeReference(flags)) {
      return flags &
          (openFlagNodeReference | openFlagDirectory | openFlagDescribe);
    }
    return flags;
  }

  /// Inode number as defined in io.fidl.
  int inodeNumber();

  /// This function is called from [fuchsia.io.Directory#open].
  /// This function parses path and opens correct node.
  ///
  /// Vnode provides a simplified implementation for non-directory types.
  /// Behavior:
  /// For directory types, it will throw UnimplementedError error.
  /// For non empty path it will fail with [ERR_NOT_DIR].
  void open(int flags, int mode, String path, InterfaceRequest<Node> request,
      [int parentFlags = Flags.fsRights]) {
    if (type() == direntTypeDirectory) {
      // dir types should implement this function
      throw UnimplementedError();
    }
    sendErrorEvent(flags, ZX.ERR_NOT_DIR, request);
  }

  /// Create a error node to send onOpen event with failure status.
  void sendErrorEvent(int flags, int status, InterfaceRequest<Node> request) {
    if ((flags & openFlagDescribe) != 0) {
      var e = ErrorNodeForSendingEvent(status, _removeErrorNode, request);
      _errorNodes.add(e);
    } else {
      request.close();
    }
  }

  /// Should be one of
  ///
  /// direntTypeUnknown
  /// direntTypeDirectory
  /// direntTypeBlockDevice
  /// direntTypeFile
  /// direntTypeSocket
  /// direntTypeService
  ///
  /// These are defined in io.fidl.
  int type();

  void _removeErrorNode(ErrorNodeForSendingEvent e) {
    _errorNodes.remove(e);
  }
}
