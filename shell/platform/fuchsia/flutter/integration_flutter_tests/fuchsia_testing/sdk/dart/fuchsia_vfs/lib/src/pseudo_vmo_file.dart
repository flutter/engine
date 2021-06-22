// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:zircon/zircon.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';

import 'pseudo_file.dart';

// ignore_for_file: public_member_api_docs

typedef VmoFn = Vmo? Function();

/// A [PseudoVmoFile] is a [VmoFile] typed [PseudoFile] whose content is read
/// from a [Vmo] dynamically produced by a supplied callback.
///
/// Each FIDL connection to a [PseudoVmoFile] calls the supplied callback once
/// and reads the content of the produced [Vmo] into a buffer. Therefore,
/// connection order is important.
///
/// Reads on each connection are seperately buffered.
class PseudoVmoFile extends PseudoFile {
  final VmoFn? _vmoFn;

  /// Constructor for read-only [Vmo]
  ///
  /// Throws Exception if _vmoFn is null.
  ///
  /// Resulting PseudoVmoFile returns nothing when read as a regular file.
  PseudoVmoFile.readOnly(this._vmoFn) : super.readOnly(() => Uint8List(0)) {
    ArgumentError.checkNotNull(_vmoFn, 'Vmo Function');
  }

  /// Describes this node and exposes a duplicate of the underlying Vmo.
  ///
  /// Returns null when vmoFn returns null or duplicate fails.
  ///
  /// The function calls the passed callback.
  @override
  NodeInfo describe() {
    final Vmo? originalVmo = _vmoFn!();
    final Vmo? duplicatedVmo =
        originalVmo?.duplicate(ZX.RIGHTS_BASIC | ZX.RIGHT_READ | ZX.RIGHT_MAP);
    if (duplicatedVmo == null) {
      return NodeInfo.withFile(FileObject(event: null));
    }

    return NodeInfo.withVmofile(Vmofile(
        vmo: duplicatedVmo, offset: 0, length: duplicatedVmo.getSize().size));
  }
}
