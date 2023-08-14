// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:nativewrappers';
import 'dart:typed_data';

import 'context.dart';

/// A buffer data range.
class BufferView {
  /// The buffer of this view.
  final Buffer buffer;
  /// The start of the view, in bytes starting from the beginning of the
  /// [buffer].
  final int offsetInBytes;
  /// The end of the view
  final int lengthInBytes;

  /// Create a new view into a buffer on the GPU.
  const BufferView(
      {required this.buffer,
      required this.offsetInBytes,
      required this.lengthInBytes});
}

/// A buffer that can be referenced by commands on the GPU.
mixin Buffer {}

/// [HostBuffer] is a [Buffer] which is allocated on the host and lazily
/// uploaded to the GPU. A [HostBuffer] can be safely mutated or extended at
/// any time on the host, and will be automatically re-uploaded to the GPU
/// the next time a GPU operation needs to access it.
///
/// This is useful for efficiently chunking sparse data uploads, especially
/// ephemeral uniform data that needs to change from frame to frame.
///
/// Note: Different platforms have different data alignment requirements for
///       accessing device buffer data. The [HostBuffer] takes these
///       requirements into account and automatically inserts padding between
///       emplaced data if necessary.
class HostBuffer extends NativeFieldWrapperClass1 with Buffer {
  /// Creates a new HostBuffer.
  HostBuffer() {
    _initialize();
  }

  /// Wrap with native counterpart.
  @Native<Void Function(Handle)>(
      symbol: 'InternalFlutterGpu_HostBuffer_Initialize')
  external void _initialize();

  /// Append byte data to the end of the [HostBuffer] and produce a [BufferView]
  /// that references the new data in the buffer.
  ///
  /// This method automatically inserts padding in-between emplaced data if
  /// necessary to abide by platform-specific uniform alignment requirements.
  ///
  /// The updated buffer will be uploaded to the GPU if the returned
  /// [BufferView] is used by a rendering command.
  BufferView emplaceBytes({required ByteData bytes}) {
    int resultOffset = _emplaceBytes(bytes);
    return BufferView(
        buffer: this,
        offsetInBytes: resultOffset,
        lengthInBytes: bytes.lengthInBytes);
  }

  @Native<Uint64 Function(Pointer<Void>, Handle)>(
      symbol: 'InternalFlutterGpu_HostBuffer_EmplaceBytes')
  external int _emplaceBytes(ByteData bytes);
}
