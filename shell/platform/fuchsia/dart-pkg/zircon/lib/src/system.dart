// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of zircon;

// ignore_for_file: native_function_body_in_non_sdk_code
// ignore_for_file: public_member_api_docs

@pragma('vm:entry-point')
class _Namespace { // ignore: unused_element
  // No public constructor - this only has static methods.
  _Namespace._();

  // Library private variable set by the embedder used to cache the
  // namespace (as an fdio_ns_t*).
  @pragma('vm:entry-point')
  static int _namespace; // ignore: unused_field
}

/// An exception representing an error returned as an zx_status_t.
class ZxStatusException implements Exception {
  final String message;
  final int status;

  ZxStatusException(this.status, [this.message]);

  @override
  String toString() {
    if (message == null)
      return 'ZxStatusException: status = $status';
    else
      return 'ZxStatusException: status = $status, "$message"';
  }
}

class _Result {
  final int status;
  const _Result(this.status);
}

@pragma('vm:entry-point')
class HandleResult extends _Result {
  final Handle handle;
  @pragma('vm:entry-point')
  const HandleResult(final int status, [this.handle]) : super(status);
  @override
  String toString() => 'HandleResult(status=$status, handle=$handle)';
}

@pragma('vm:entry-point')
class HandlePairResult extends _Result {
  final Handle first;
  final Handle second;
  @pragma('vm:entry-point')
  const HandlePairResult(final int status, [this.first, this.second])
      : super(status);
  @override
  String toString() =>
      'HandlePairResult(status=$status, first=$first, second=$second)';
}

@pragma('vm:entry-point')
class ReadResult extends _Result {
  final ByteData bytes;
  final int numBytes;
  final List<Handle> handles;
  @pragma('vm:entry-point')
  const ReadResult(final int status, [this.bytes, this.numBytes, this.handles])
      : super(status);
  Uint8List bytesAsUint8List() =>
      bytes.buffer.asUint8List(bytes.offsetInBytes, numBytes);
  String bytesAsUTF8String() => utf8.decode(bytesAsUint8List());
  @override
  String toString() =>
      'ReadResult(status=$status, bytes=$bytes, numBytes=$numBytes, handles=$handles)';
}

@pragma('vm:entry-point')
class WriteResult extends _Result {
  final int numBytes;
  @pragma('vm:entry-point')
  const WriteResult(final int status, [this.numBytes]) : super(status);
  @override
  String toString() => 'WriteResult(status=$status, numBytes=$numBytes)';
}

@pragma('vm:entry-point')
class GetSizeResult extends _Result {
  final int size;
  @pragma('vm:entry-point')
  const GetSizeResult(final int status, [this.size]) : super(status);
  @override
  String toString() => 'GetSizeResult(status=$status, size=$size)';
}

@pragma('vm:entry-point')
class FromFileResult extends _Result {
  final Handle handle;
  final int numBytes;
  @pragma('vm:entry-point')
  const FromFileResult(final int status, [this.handle, this.numBytes])
      : super(status);
  @override
  String toString() =>
      'FromFileResult(status=$status, handle=$handle, numBytes=$numBytes)';
}

@pragma('vm:entry-point')
class MapResult extends _Result {
  final Uint8List data;
  @pragma('vm:entry-point')
  const MapResult(final int status, [this.data]) : super(status);
  @override
  String toString() => 'MapResult(status=$status, data=$data)';
}

@pragma('vm:entry-point')
class System extends NativeFieldWrapperClass2 {
  // No public constructor - this only has static methods.
  System._();

  // Channel operations.
  static HandlePairResult channelCreate([int options = 0])
      native 'System_channelCreate';
  static HandleResult channelFromFile(String path)
      native 'System_channelFromFile';
  static int channelWrite(Handle channel, ByteData data, List<Handle> handles)
      native 'System_channelWrite';
  static ReadResult channelQueryAndRead(Handle channel)
      native 'System_channelQueryAndRead';
  static int connectToService(String path, Handle channel)
    native 'System_connectToService';

  // Clock operations.
  static int clockGet(int clockId) native 'System_clockGet';

  // Eventpair operations.
  static HandlePairResult eventpairCreate([int options = 0])
      native 'System_eventpairCreate';

  // Socket operations.
  static HandlePairResult socketCreate([int options = 0])
      native 'System_socketCreate';
  static WriteResult socketWrite(Handle socket, ByteData data, int options)
      native 'System_socketWrite';
  static ReadResult socketRead(Handle socket, int size)
      native 'System_socketRead';

  // Vmo operations.
  static HandleResult vmoCreate(int size, [int options = 0])
      native 'System_vmoCreate';
  static FromFileResult vmoFromFile(String path) native 'System_vmoFromFile';
  static GetSizeResult vmoGetSize(Handle vmo) native 'System_vmoGetSize';
  static int vmoSetSize(Handle vmo, int size) native 'System_vmoSetSize';
  static int vmoWrite(Handle vmo, int offset, ByteData bytes)
      native 'System_vmoWrite';
  static ReadResult vmoRead(Handle vmo, int offset, int size)
      native 'System_vmoRead';
  static MapResult vmoMap(Handle vmo) native 'System_vmoMap';
}
