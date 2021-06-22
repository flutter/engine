// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:zircon/zircon.dart';
import 'package:fidl_fuchsia_io/fidl_async.dart';

import 'pseudo_file.dart';

// ignore_for_file: unnecessary_null_comparison

/// Specifies how a VMO wrapped by [VmoFile] may be shared.
enum VmoSharingMode {
  /// The VMO may not be shared, the VMO file appears as a regular file.
  noSharing,

  /// A duplicate of the VMO is shared.
  shareDuplicate,

  // TODO(crjohns): Support cloning.
}

/// A node which wraps a VMO that can be duplicated when opened.
///
/// Reads when opening a [VmoFile] as a file are buffered, and repeated reads
/// will obtain the same information unless the connection is closed between
/// reads.
/// A duplicate of the underlying VMO is exposed through [Node.Describe] when
/// [VmoSharingMode] is [shareDuplicate].
class VmoFile extends PseudoFile {
  final Vmo _vmo;
  final VmoSharingMode _sharingMode;

  /// Constructor for read-only [Vmo]
  VmoFile.readOnly(this._vmo, this._sharingMode)
      : assert(_vmo != null),
        super.readOnly(() {
          int size = _vmo.getSize().size;
          return _vmo.read(size).bytesAsUint8List();
        }) {
    if (_vmo == null) {
      throw Exception('Vmo cannot be null');
    }
  }

  // TODO(crjohns): Support writable vmo files.

  /// Describes this node. Returns null on error.
  @override
  NodeInfo describe() {
    if (_sharingMode == VmoSharingMode.shareDuplicate) {
      final Vmo duplicatedVmo =
          _vmo.duplicate(ZX.RIGHTS_BASIC | ZX.RIGHT_READ | ZX.RIGHT_MAP);

      if (duplicatedVmo != null) {
        return NodeInfo.withVmofile(Vmofile(
            vmo: duplicatedVmo, offset: 0, length: _vmo.getSize().size));
      }
    }

    return NodeInfo.withFile(FileObject(event: null));
  }
}
