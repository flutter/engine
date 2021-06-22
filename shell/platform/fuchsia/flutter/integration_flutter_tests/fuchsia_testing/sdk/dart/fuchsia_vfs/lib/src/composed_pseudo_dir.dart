// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:fidl/fidl.dart' as fidl;
import 'package:fidl_fuchsia_io/fidl_async.dart' as fidl_io;
import 'package:meta/meta.dart';
import 'package:zircon/zircon.dart';

import 'internal/_flags.dart';
import 'pseudo_dir.dart';
import 'vnode.dart';

// ignore_for_file: unnecessary_null_comparison, unused_import

/// A [PseudoDir] which will pass requests for inherited nodes to the underlying
/// [fidl_io.Directory].
///
/// Inherited Nodes must be specified at time of creation. Any requests to open
/// nodes in the [inheritedNodes] list will be proxied to the provided
/// directory. If a request is made to a node not included in this list it will
/// attempt to open the node on the super class.
class ComposedPseudoDir extends PseudoDir {
  final List<String> _inheritedNodes;

  final fidl_io.Directory _directory;

  /// Constructs the [ComposedPseudoDir] and provides a list
  /// of [inheritedServices] to proxy to the [directory].
  ComposedPseudoDir({
    required fidl_io.Directory directory,
    List<String> inheritedNodes = const [],
  })  : assert(inheritedNodes != null),
        assert(directory != null),
        _inheritedNodes = List.of(inheritedNodes),
        _directory = directory;

  @override
  void open(int flags, int mode, String path,
      fidl.InterfaceRequest<fidl_io.Node> request,
      [int parentFlags = Flags.fsRights]) {
    if (_inheritedNodes.contains(path)) {
      _directory.open(flags, mode, path, request);
    } else {
      super.open(flags, mode, path, request, parentFlags);
    }
  }

  @override
  int addNode(String name, Vnode node) {
    if (_inheritedNodes.contains(name)) {
      return ZX.ERR_ALREADY_EXISTS;
    } else {
      return super.addNode(name, node);
    }
  }
}
