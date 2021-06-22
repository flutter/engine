// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';

import 'package:zircon/zircon.dart';

import 'bits.dart';
import 'codec.dart';
import 'enum.dart';
import 'error.dart';
import 'interface.dart';
import 'struct.dart';
import 'table.dart';
import 'unknown_data.dart';
import 'xunion.dart';

// ignore_for_file: public_member_api_docs
// ignore_for_file: always_specify_types

const _notNullable = FidlError('Found null for a non-nullable type',
    FidlErrorCode.fidlNonNullableTypeWithNullValue);

void _throwIfExceedsLimit(int count, int? limit) {
  if (limit != null && count > limit) {
    throw FidlError('Found an object wth $count elements. Limited to $limit.',
        FidlErrorCode.fidlStringTooLong);
  }
}

void _throwIfCountMismatch(int count, int expectedCount) {
  if (count != expectedCount) {
    throw FidlError('Found an array of count $count. Expected $expectedCount.');
  }
}

// Threshold at which _copy* functions switch from a for loop for copying to
// setRange().
// For small loops, constructing a typed view and calling setRange can take
// twice as long as directly iterating over the list.
// This threshold is chosen somewhat arbitrarily based on partial data
// and may not be optimal.
const _copySetRangeThreshold = 64;

void _copyInt8(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    for (final element in value) {
      data.setInt8(off, element);
      off++;
    }
  } else {
    data.buffer.asInt8List(offset).setRange(0, value.length, value);
  }
}

void _copyUint8(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    for (final element in value) {
      data.setUint8(off, element);
      off++;
    }
  } else {
    data.buffer.asUint8List(offset).setRange(0, value.length, value);
  }
}

void _copyInt16(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 2;
    for (final element in value) {
      data.setInt16(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asInt16List(offset).setRange(0, value.length, value);
  }
}

void _copyUint16(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 2;
    for (final element in value) {
      data.setUint16(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asUint16List(offset).setRange(0, value.length, value);
  }
}

void _copyInt32(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 4;
    for (final element in value) {
      data.setInt32(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asInt32List(offset).setRange(0, value.length, value);
  }
}

void _copyUint32(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 4;
    for (final element in value) {
      data.setUint32(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asUint32List(offset).setRange(0, value.length, value);
  }
}

void _copyInt64(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 8;
    for (final element in value) {
      data.setInt64(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asInt64List(offset).setRange(0, value.length, value);
  }
}

void _copyUint64(ByteData data, Iterable<int> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 8;
    for (final element in value) {
      data.setUint64(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asUint64List(offset).setRange(0, value.length, value);
  }
}

void _copyFloat32(ByteData data, Iterable<double> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 4;
    for (final element in value) {
      data.setFloat32(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asFloat32List(offset).setRange(0, value.length, value);
  }
}

void _copyFloat64(ByteData data, Iterable<double> value, int offset) {
  if (value.length < _copySetRangeThreshold) {
    int off = offset;
    const int stride = 8;
    for (final element in value) {
      data.setFloat64(off, element, Endian.little);
      off += stride;
    }
  } else {
    data.buffer.asFloat64List(offset).setRange(0, value.length, value);
  }
}

const int kAllocAbsent = 0;
const int kAllocPresent = 0xFFFFFFFFFFFFFFFF;
const int kHandleAbsent = 0;
const int kHandlePresent = 0xFFFFFFFF;

abstract class FidlType<T, I extends Iterable<T>> {
  const FidlType({required this.inlineSize});

  final int inlineSize;

  int encodingInlineSize() => inlineSize;
  int decodingInlineSize() => inlineSize;

  void encode(Encoder encoder, T value, int offset, int depth);

  void encodeArray(Encoder encoder, Iterable<T> value, int offset, int depth) {
    int off = offset;
    final int stride = encodingInlineSize();
    for (final element in value) {
      encode(encoder, element, off, depth);
      off += stride;
    }
  }

  T decode(Decoder decoder, int offset, int depth);

  I decodeArray(Decoder decoder, int count, int offset, int depth);
}

abstract class SimpleFidlType<T> extends FidlType<T, List<T>> {
  const SimpleFidlType({required int inlineSize})
      : super(inlineSize: inlineSize);

  @override
  List<T> decodeArray(Decoder decoder, int count, int offset, int depth) =>
      List<T>.generate(count,
          (int i) => decode(decoder, offset + i * decodingInlineSize(), depth));
}

/// This encodes/decodes the UnknowRawData assuming it is in an envelope, i.e.
/// payload bytes followed directly by handles.
class UnknownRawDataType extends SimpleFidlType<UnknownRawData> {
  const UnknownRawDataType({required this.numBytes, required this.numHandles})
      : super(inlineSize: numBytes);

  final int numBytes;
  final int numHandles;

  @override
  void encode(Encoder encoder, UnknownRawData value, int offset, int depth) {
    _copyUint8(encoder.data, value.data, offset);
    for (int i = 0; i < value.handles.length; i++) {
      encoder.addHandle(value.handles[i]);
    }
  }

  @override
  UnknownRawData decode(Decoder decoder, int offset, int depth) {
    final Uint8List data = Uint8List(numBytes);
    for (var i = 0; i < numBytes; i++) {
      data[i] = decoder.decodeUint8(offset + i);
    }
    final handles =
        List<Handle>.generate(numHandles, (int i) => decoder.claimHandle());
    return UnknownRawData(data, handles);
  }
}

class BoolType extends SimpleFidlType<bool> {
  const BoolType() : super(inlineSize: 1);

  @override
  void encode(Encoder encoder, bool value, int offset, int depth) {
    encoder.encodeBool(value, offset);
  }

  @override
  bool decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeBool(offset);
}

class StatusType extends Int32Type {
  const StatusType();
}

class Int8Type extends FidlType<int, Int8List> {
  const Int8Type() : super(inlineSize: 1);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeInt8(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeInt8(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyInt8(encoder.data, value, offset);
  }

  @override
  Int8List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asInt8List(offset, count);
  }
}

class Int16Type extends FidlType<int, Int16List> {
  const Int16Type() : super(inlineSize: 2);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeInt16(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeInt16(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyInt16(encoder.data, value, offset);
  }

  @override
  Int16List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asInt16List(offset, count);
  }
}

class Int32Type extends FidlType<int, Int32List> {
  const Int32Type() : super(inlineSize: 4);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeInt32(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeInt32(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyInt32(encoder.data, value, offset);
  }

  @override
  Int32List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asInt32List(offset, count);
  }
}

class Int64Type extends FidlType<int, Int64List> {
  const Int64Type() : super(inlineSize: 8);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeInt64(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeInt64(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyInt64(encoder.data, value, offset);
  }

  @override
  Int64List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asInt64List(offset, count);
  }
}

class Uint8Type extends FidlType<int, Uint8List> {
  const Uint8Type() : super(inlineSize: 1);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeUint8(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeUint8(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyUint8(encoder.data, value, offset);
  }

  @override
  Uint8List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asUint8List(offset, count);
  }
}

class Uint16Type extends FidlType<int, Uint16List> {
  const Uint16Type() : super(inlineSize: 2);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeUint16(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeUint16(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyUint16(encoder.data, value, offset);
  }

  @override
  Uint16List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asUint16List(offset, count);
  }
}

class Uint32Type extends FidlType<int, Uint32List> {
  const Uint32Type() : super(inlineSize: 4);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeUint32(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeUint32(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyUint32(encoder.data, value, offset);
  }

  @override
  Uint32List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asUint32List(offset, count);
  }
}

class Uint64Type extends FidlType<int, Uint64List> {
  const Uint64Type() : super(inlineSize: 8);

  @override
  void encode(Encoder encoder, int value, int offset, int depth) {
    encoder.encodeUint64(value, offset);
  }

  @override
  int decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeUint64(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<int> value, int offset, int depth) {
    _copyUint64(encoder.data, value, offset);
  }

  @override
  Uint64List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asUint64List(offset, count);
  }
}

class Float32Type extends FidlType<double, Float32List> {
  const Float32Type() : super(inlineSize: 4);

  @override
  void encode(Encoder encoder, double value, int offset, int depth) {
    encoder.encodeFloat32(value, offset);
  }

  @override
  double decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeFloat32(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<double> value, int offset, int depth) {
    _copyFloat32(encoder.data, value, offset);
  }

  @override
  Float32List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asFloat32List(offset, count);
  }
}

class Float64Type extends FidlType<double, Float64List> {
  const Float64Type() : super(inlineSize: 8);

  @override
  void encode(Encoder encoder, double value, int offset, int depth) {
    encoder.encodeFloat64(value, offset);
  }

  @override
  double decode(Decoder decoder, int offset, int depth) =>
      decoder.decodeFloat64(offset);

  @override
  void encodeArray(
      Encoder encoder, Iterable<double> value, int offset, int depth) {
    _copyFloat64(encoder.data, value, offset);
  }

  @override
  Float64List decodeArray(Decoder decoder, int count, int offset, int depth) {
    return decoder.data.buffer.asFloat64List(offset, count);
  }
}

void _encodeHandle(Encoder encoder, Handle? value, int offset, bool nullable) {
  final bool present = value != null && value.isValid;
  if (!nullable && !present) {
    throw _notNullable;
  }
  encoder.encodeUint32(present ? kHandlePresent : kHandleAbsent, offset);
  if (present) {
    encoder.addHandle(value);
  }
}

Handle? _decodeNullableHandle(Decoder decoder, int offset) {
  final int encoded = decoder.decodeUint32(offset);
  if (encoded != kHandleAbsent && encoded != kHandlePresent) {
    throw FidlError('Invalid handle encoding: $encoded.');
  }
  return encoded == kHandlePresent ? decoder.claimHandle() : null;
}

Handle _decodeHandle(Decoder decoder, int offset) {
  final handle = _decodeNullableHandle(decoder, offset);
  if (handle == null) {
    throw _notNullable;
  }
  return handle;
}

// TODO(pascallouis): By having _HandleWrapper exported, we could DRY this code
// by simply having an AbstractHandleType<H extend HandleWrapper<H>> and having
// the encoding / decoding once, with the only specialization on a per-type
// basis being construction.
// Further, if each HandleWrapper were to offer a static ctor function to invoke
// their constrctors, could be called directly.
// We could also explore having a Handle be itself a subtype of HandleWrapper
// to further standardize handling of handles.

abstract class _BaseHandleType<W> extends SimpleFidlType<W> {
  const _BaseHandleType({required this.objectType, required this.rights})
      : super(inlineSize: 4);

  final int objectType;
  final int rights;

  W wrap(Handle handle);
  Handle? unwrap(W wrapper);

  @override
  void encode(Encoder encoder, W value, int offset, int depth) =>
      _encodeHandle(encoder, unwrap(value), offset, false);

  @override
  W decode(Decoder decoder, int offset, int depth) =>
      wrap(_decodeHandle(decoder, offset));
}

class NullableHandleType<W> extends SimpleFidlType<W?> {
  final _BaseHandleType<W> _base;
  const NullableHandleType(this._base) : super(inlineSize: 4);

  @override
  void encode(Encoder encoder, W? value, int offset, int depth) =>
      _encodeHandle(
          encoder, value == null ? null : _base.unwrap(value), offset, true);

  @override
  W? decode(Decoder decoder, int offset, int depth) {
    final Handle? handle = _decodeNullableHandle(decoder, offset);
    return handle == null ? null : _base.wrap(handle);
  }
}

class HandleType extends _BaseHandleType<Handle> {
  const HandleType({required objectType, required rights})
      : super(objectType: objectType, rights: rights);

  @override
  Handle wrap(Handle handle) => handle;

  @override
  Handle? unwrap(Handle handle) => handle;
}

class ChannelType extends _BaseHandleType<Channel> {
  const ChannelType({required objectType, required rights})
      : super(objectType: objectType, rights: rights);

  @override
  Channel wrap(Handle handle) => Channel(handle);
  @override
  Handle? unwrap(Channel wrapper) => wrapper.handle;
}

class EventPairType extends _BaseHandleType<EventPair> {
  const EventPairType({required objectType, required rights})
      : super(objectType: objectType, rights: rights);

  @override
  EventPair wrap(Handle handle) => EventPair(handle);
  @override
  Handle? unwrap(EventPair wrapper) => wrapper.handle;
}

class SocketType extends _BaseHandleType<Socket> {
  const SocketType({required objectType, required rights})
      : super(objectType: objectType, rights: rights);

  @override
  Socket wrap(Handle handle) => Socket(handle);
  @override
  Handle? unwrap(Socket wrapper) => wrapper.handle;
}

class VmoType extends _BaseHandleType<Vmo> {
  const VmoType({required objectType, required rights})
      : super(objectType: objectType, rights: rights);

  @override
  Vmo wrap(Handle handle) => Vmo(handle);
  @override
  Handle? unwrap(Vmo wrapper) => wrapper.handle;
}

class InterfaceHandleType<T> extends SimpleFidlType<InterfaceHandle<T>> {
  const InterfaceHandleType() : super(inlineSize: 4);

  @override
  void encode(
          Encoder encoder, InterfaceHandle<T> value, int offset, int depth) =>
      _encodeHandle(encoder, value.channel?.handle, offset, false);

  @override
  InterfaceHandle<T> decode(Decoder decoder, int offset, int depth) =>
      InterfaceHandle<T>(Channel(_decodeHandle(decoder, offset)));
}

class NullableInterfaceHandleType<T>
    extends SimpleFidlType<InterfaceHandle<T>?> {
  const NullableInterfaceHandleType() : super(inlineSize: 4);

  @override
  void encode(
          Encoder encoder, InterfaceHandle<T>? value, int offset, int depth) =>
      _encodeHandle(
          encoder, value == null ? null : value.channel?.handle, offset, true);

  @override
  InterfaceHandle<T>? decode(Decoder decoder, int offset, int depth) {
    final Handle? handle = _decodeNullableHandle(decoder, offset);
    return handle == null ? null : InterfaceHandle<T>(Channel(handle));
  }
}

class InterfaceRequestType<T> extends SimpleFidlType<InterfaceRequest<T>> {
  const InterfaceRequestType() : super(inlineSize: 4);

  @override
  void encode(
          Encoder encoder, InterfaceRequest<T> value, int offset, int depth) =>
      _encodeHandle(encoder, value.channel?.handle, offset, false);

  @override
  InterfaceRequest<T> decode(Decoder decoder, int offset, int depth) =>
      InterfaceRequest<T>(Channel(_decodeHandle(decoder, offset)));
}

class NullableInterfaceRequestType<T>
    extends SimpleFidlType<InterfaceRequest<T>?> {
  const NullableInterfaceRequestType() : super(inlineSize: 4);

  @override
  void encode(
          Encoder encoder, InterfaceRequest<T>? value, int offset, int depth) =>
      _encodeHandle(
          encoder, value == null ? null : value.channel?.handle, offset, true);

  @override
  InterfaceRequest<T>? decode(Decoder decoder, int offset, int depth) {
    final Handle? handle = _decodeNullableHandle(decoder, offset);
    return handle == null ? null : InterfaceRequest<T>(Channel(handle));
  }
}

void _encodeString(Encoder encoder, String value, int offset, int depth,
    int? maybeElementCount) {
  final bytes = Utf8Encoder().convert(value);
  final int size = bytes.length;
  _throwIfExceedsLimit(size, maybeElementCount);
  encoder
    ..encodeUint64(size, offset) // size
    ..encodeUint64(kAllocPresent, offset + 8); // data
  int childOffset = encoder.alloc(size, depth);
  _copyUint8(encoder.data, bytes, childOffset);
}

String? _decodeString(Decoder decoder, int offset, int depth) {
  final int size = decoder.decodeUint64(offset);
  final int data = decoder.decodeUint64(offset + 8);
  if (data == kAllocAbsent) {
    if (size > 0) {
      throw FidlError('Expected string, received null',
          FidlErrorCode.fidlNonEmptyStringWithNullBody);
    }
    return null;
  }
  final Uint8List bytes =
      decoder.data.buffer.asUint8List(decoder.claimMemory(size, depth), size);
  try {
    return const Utf8Decoder().convert(bytes, 0, size);
  } on FormatException {
    throw FidlError('Received a string with invalid UTF8: $bytes');
  }
}

void _encodeNullVector(Encoder encoder, int offset) {
  encoder
    ..encodeUint64(0, offset) // size
    ..encodeUint64(kAllocAbsent, offset + 8); // data
}

class StringType extends SimpleFidlType<String> {
  const StringType({
    this.maybeElementCount,
  }) : super(inlineSize: 16);

  final int? maybeElementCount;

  // See fidl_string_t.

  @override
  void encode(Encoder encoder, String value, int offset, int depth) {
    _encodeString(encoder, value, offset, depth, maybeElementCount);
  }

  @override
  String decode(Decoder decoder, int offset, int depth) {
    String? value = _decodeString(decoder, offset, depth);
    if (value == null) {
      throw _notNullable;
    }
    _throwIfExceedsLimit(value.length, maybeElementCount);
    return value;
  }
}

class NullableStringType extends SimpleFidlType<String?> {
  const NullableStringType({
    this.maybeElementCount,
  }) : super(inlineSize: 16);

  final int? maybeElementCount;
  @override
  void encode(Encoder encoder, String? value, int offset, int depth) {
    if (value == null) {
      _encodeNullVector(encoder, offset);
    } else {
      _encodeString(encoder, value, offset, depth, maybeElementCount);
    }
  }

  @override
  String? decode(Decoder decoder, int offset, int depth) {
    String? value = _decodeString(decoder, offset, depth);
    if (value != null) {
      _throwIfExceedsLimit(value.length, maybeElementCount);
    }
    return value;
  }
}

class PointerType<T> extends SimpleFidlType<T?> {
  const PointerType({required this.element}) : super(inlineSize: 8);

  final FidlType element;

  @override
  void encode(Encoder encoder, T? value, int offset, int depth) {
    if (value == null) {
      encoder.encodeUint64(kAllocAbsent, offset);
    } else {
      encoder.encodeUint64(kAllocPresent, offset);
      int childOffset = encoder.alloc(element.encodingInlineSize(), depth);
      element.encode(encoder, value, childOffset, depth + 1);
    }
  }

  @override
  T? decode(Decoder decoder, int offset, int depth) {
    final int data = decoder.decodeUint64(offset);
    validateEncoded(data);
    if (data == kAllocAbsent) {
      return null;
    }
    T? decoded = element.decode(decoder,
        decoder.claimMemory(element.decodingInlineSize(), depth), depth + 1);
    return decoded;
  }

  void validateEncoded(int encoded) {
    if (encoded != kAllocAbsent && encoded != kAllocPresent) {
      throw FidlError('Invalid pointer encoding: $encoded.');
    }
  }
}

class MemberType<T> {
  const MemberType({
    required this.type,
    required this.offset,
  });

  final FidlType type;
  final int offset;

  void encode(Encoder encoder, T value, int base, int depth) {
    type.encode(encoder, value, base + offset, depth);
  }

  T decode(Decoder decoder, int base, int depth) {
    return type.decode(decoder, base + offset, depth);
  }
}

class StructType<T extends Struct> extends SimpleFidlType<T> {
  const StructType({
    required int inlineSize,
    required this.structDecode,
  }) : super(inlineSize: inlineSize);

  final StructDecode<T> structDecode;

  @override
  void encode(Encoder encoder, T value, int offset, int depth) {
    value.$encode(encoder, offset, depth);
  }

  @override
  T decode(Decoder decoder, int offset, int depth) {
    return structDecode(decoder, offset, depth);
  }
}

const int _kEnvelopeSize = 16;

void _encodeEnvelopePresent<T, I extends Iterable<T>>(
    Encoder encoder, int offset, int depth, T field, FidlType<T, I> fieldType) {
  int numHandles = encoder.countHandles();
  final fieldOffset = encoder.alloc(fieldType.encodingInlineSize(), depth);
  fieldType.encode(encoder, field, fieldOffset, depth + 1);
  numHandles = encoder.countHandles() - numHandles;
  final numBytes = encoder.nextOffset() - fieldOffset;

  encoder
    ..encodeUint32(numBytes, offset)
    ..encodeUint32(numHandles, offset + 4)
    ..encodeUint64(kAllocPresent, offset + 8);
}

void _encodeEnvelopeAbsent(Encoder encoder, int offset) {
  encoder..encodeUint64(0, offset)..encodeUint64(kAllocAbsent, offset + 8);
}

class EnvelopeHeader {
  int numBytes;
  int numHandles;
  int fieldPresent;
  EnvelopeHeader(this.numBytes, this.numHandles, this.fieldPresent);
}

T? _decodeEnvelope<T>(
    Decoder decoder, int offset, int depth, SimpleFidlType<T>? fieldType,
    {bool isEmpty = false}) {
  final header = _decodeEnvelopeHeader(decoder, offset);
  return _decodeEnvelopeContent(decoder, header, fieldType, depth, isEmpty);
}

EnvelopeHeader _decodeEnvelopeHeader(Decoder decoder, int offset) {
  return EnvelopeHeader(
    decoder.decodeUint32(offset),
    decoder.decodeUint32(offset + 4),
    decoder.decodeUint64(offset + 8),
  );
}

T? _decodeEnvelopeContent<T, I extends Iterable<T>>(Decoder decoder,
    EnvelopeHeader header, FidlType<T, I>? fieldType, int depth, bool isEmpty) {
  switch (header.fieldPresent) {
    case kAllocPresent:
      if (isEmpty) throw FidlError('expected empty envelope');
      if (fieldType != null) {
        final fieldOffset =
            decoder.claimMemory(fieldType.decodingInlineSize(), depth);
        final claimedHandles = decoder.countClaimedHandles();
        final field = fieldType.decode(decoder, fieldOffset, depth + 1);
        final numBytesConsumed = decoder.nextOffset() - fieldOffset;
        final numHandlesConsumed =
            decoder.countClaimedHandles() - claimedHandles;

        if (header.numHandles != numHandlesConsumed) {
          throw FidlError('envelope handles were mis-sized',
              FidlErrorCode.fidlInvalidNumHandlesInEnvelope);
        }
        if (header.numBytes != numBytesConsumed) {
          throw FidlError('envelope was mis-sized',
              FidlErrorCode.fidlInvalidNumBytesInEnvelope);
        }
        return field;
      }

      decoder.claimMemory(header.numBytes, depth);
      for (int i = 0; i < header.numHandles; i++) {
        final handle = decoder.claimHandle();
        try {
          handle.close();
          // ignore: avoid_catches_without_on_clauses
        } catch (e) {
          // best effort
        }
      }
      return null;
    case kAllocAbsent:
      if (header.numBytes != 0)
        throw FidlError('absent envelope with non-zero bytes',
            FidlErrorCode.fidlInvalidNumBytesInEnvelope);
      if (header.numHandles != 0)
        throw FidlError('absent envelope with non-zero handles',
            FidlErrorCode.fidlInvalidNumHandlesInEnvelope);
      return null;
    default:
      throw FidlError(
          'Bad reference encoding', FidlErrorCode.fidlInvalidPresenceIndicator);
  }
}

void _maybeThrowOnUnknownHandles(bool resource, UnknownRawData data) {
  if (!resource && data.handles.isNotEmpty) {
    data.closeHandles();
    throw FidlError('Unknown data contained handles on encode',
        FidlErrorCode.fidlNonResourceHandle);
  }
}

class TableType<T extends Table> extends SimpleFidlType<T> {
  const TableType({
    required int inlineSize,
    required this.members,
    required this.ctor,
    required this.resource,
  }) : super(inlineSize: inlineSize);

  final List<FidlType?> members;
  final TableFactory<T> ctor;
  final bool resource;

  @override
  void encode(Encoder encoder, T value, int offset, int depth) {
    final unknownDataMap = value.$unknownData;

    // Determining max index
    int maxIndex = -1;
    for (int i = 0; i < members.length; i++) {
      final field = value.$field(i);
      if (field != null) {
        maxIndex = i;
      }
    }
    if (unknownDataMap != null) {
      // Update the max index based on the table's unknown fields. The unknown
      // data map is keyed by ordinal, not index, so subtract by one.
      maxIndex = max(maxIndex, unknownDataMap.keys.fold(0, max) - 1);
    }
    int maxOrdinal = maxIndex + 1;

    // Header.
    encoder
      ..encodeUint64(maxOrdinal, offset)
      ..encodeUint64(kAllocPresent, offset + 8);

    // Sizing
    int envelopeOffset = encoder.alloc(maxOrdinal * _kEnvelopeSize, depth);

    // Envelopes, and fields.
    for (int i = 0; i <= maxIndex; i++) {
      var field = value.$field(i);
      FidlType? fieldType;
      if (i < members.length) {
        fieldType = members[i];
      }
      if (fieldType == null && unknownDataMap != null) {
        // .$field is accessed by index, whereas the unknown data map is
        // accessed by ordinal
        final unknownData = unknownDataMap[i + 1];
        if (unknownData != null) {
          _maybeThrowOnUnknownHandles(resource, unknownData);
          field = unknownData;
          fieldType = UnknownRawDataType(
              numBytes: unknownData.data.length,
              numHandles: unknownData.handles.length);
        }
      }

      if (field != null && fieldType != null) {
        _encodeEnvelopePresent(
            encoder, envelopeOffset, depth + 1, field, fieldType);
      } else {
        _encodeEnvelopeAbsent(encoder, envelopeOffset);
      }
      envelopeOffset += _kEnvelopeSize;
    }
  }

  @override
  T decode(Decoder decoder, int offset, int depth) {
    // Header.
    final int maxOrdinal = decoder.decodeUint64(offset);
    final int data = decoder.decodeUint64(offset + 8);
    switch (data) {
      case kAllocPresent:
        break; // good
      case kAllocAbsent:
        throw FidlError('Unexpected null reference',
            FidlErrorCode.fidlNonNullableTypeWithNullValue);
      default:
        throw FidlError('Bad reference encoding',
            FidlErrorCode.fidlInvalidPresenceIndicator);
    }

    // Early exit on empty table.
    if (maxOrdinal == 0) {
      return ctor({});
    }

    // Offsets.
    int envelopeOffset =
        decoder.claimMemory(maxOrdinal * _kEnvelopeSize, depth);

    // Envelopes, and fields.
    final Map<int, dynamic> argv = {};
    Map<int, UnknownRawData> unknownData = {};
    for (int ordinal = 1; ordinal <= maxOrdinal; ordinal++) {
      FidlType? fieldType;
      if (ordinal <= members.length) {
        fieldType = members[ordinal - 1];
      } else {
        fieldType = null;
      }

      final header = _decodeEnvelopeHeader(decoder, envelopeOffset);
      final field = _decodeEnvelopeContent(
          decoder,
          header,
          fieldType ??
              UnknownRawDataType(
                  numBytes: header.numBytes, numHandles: header.numHandles),
          depth + 1,
          false);
      if (field != null) {
        if (fieldType != null) {
          argv[ordinal] = field;
        } else {
          _maybeThrowOnUnknownHandles(resource, field);
          unknownData[ordinal] = field;
        }
      }
      envelopeOffset += _kEnvelopeSize;
    }

    return ctor(argv, unknownData);
  }
}

void _encodeUnion<T extends XUnion>(Encoder encoder, T value, int offset,
    int depth, Map<int, FidlType> members, bool flexible, bool resource) {
  final int envelopeOffset = offset + 8;
  final int ordinal = value.$ordinal;
  var fieldType = members[ordinal];
  final data = value.$data;
  if (fieldType == null && flexible && data is UnknownRawData) {
    _maybeThrowOnUnknownHandles(resource, data);
    fieldType = UnknownRawDataType(
        numBytes: data.data.length, numHandles: data.handles.length);
  }
  if (fieldType == null)
    throw FidlError('Bad xunion ordinal: $ordinal',
        FidlErrorCode.fidlStrictXUnionUnknownField);

  encoder.encodeUint64(ordinal, offset);
  _encodeEnvelopePresent(encoder, envelopeOffset, depth, data, fieldType);
}

T? _decodeUnion<T extends XUnion>(
    Decoder decoder,
    int offset,
    int depth,
    Map<int, FidlType> members,
    XUnionFactory<T> ctor,
    bool flexible,
    bool resource) {
  final int envelopeOffset = offset + 8;
  final int ordinal = decoder.decodeUint64(offset);
  if (ordinal == 0) {
    _decodeEnvelope(decoder, envelopeOffset, depth, null, isEmpty: true);
    return null;
  } else {
    final header = _decodeEnvelopeHeader(decoder, envelopeOffset);
    var fieldType = members[ordinal];
    if (fieldType == null) {
      final unknownData = _decodeEnvelopeContent(
          decoder,
          header,
          UnknownRawDataType(
              numBytes: header.numBytes, numHandles: header.numHandles),
          depth,
          false);
      if (unknownData == null) throw FidlError('Bad xunion: missing content');

      if (!flexible) {
        unknownData.closeHandles();
        throw FidlError('Bad xunion ordinal: $ordinal',
            FidlErrorCode.fidlStrictXUnionUnknownField);
      }

      _maybeThrowOnUnknownHandles(resource, unknownData);
      return ctor(ordinal, unknownData);
    }
    final field =
        _decodeEnvelopeContent(decoder, header, fieldType, depth, false);
    if (field == null) throw FidlError('Bad xunion: missing content');
    return ctor(ordinal, field);
  }
}

class UnionType<T extends XUnion> extends SimpleFidlType<T> {
  const UnionType(
      {required this.members,
      required this.ctor,
      required this.flexible,
      required this.resource})
      : super(inlineSize: 24);

  final Map<int, FidlType> members;
  final XUnionFactory<T> ctor;
  final bool flexible;
  final bool resource;

  @override
  void encode(Encoder encoder, T value, int offset, int depth) =>
      _encodeUnion(encoder, value, offset, depth, members, flexible, resource);

  @override
  T decode(Decoder decoder, int offset, int depth) {
    T? value =
        _decodeUnion(decoder, offset, depth, members, ctor, flexible, resource);
    if (value == null) {
      throw _notNullable;
    }
    return value;
  }
}

class NullableUnionType<T extends XUnion> extends SimpleFidlType<T?> {
  const NullableUnionType(
      {required this.members,
      required this.ctor,
      required this.flexible,
      required this.resource})
      : super(inlineSize: 24);

  final Map<int, FidlType> members;
  final XUnionFactory<T> ctor;
  final bool flexible;
  final bool resource;

  @override
  void encode(Encoder encoder, T? value, int offset, int depth) {
    if (value == null) {
      encoder.encodeUint64(0, offset);
      _encodeEnvelopeAbsent(encoder, offset + 8);
    } else {
      _encodeUnion(encoder, value, offset, depth, members, flexible, resource);
    }
  }

  @override
  T? decode(Decoder decoder, int offset, int depth) =>
      _decodeUnion(decoder, offset, depth, members, ctor, flexible, resource);
}

class EnumType<T extends Enum> extends SimpleFidlType<T> {
  const EnumType({
    required this.type,
    required this.values,
    required this.ctor,
  }) : super(inlineSize: 0); // Note: inlineSize is calculated below

  final FidlType<int, Iterable<int>> type;
  // TODO(fxb/7644): Currently, types are generated as consts. This means that
  // we cannot use a Set, since there is no const Set. What we want to do
  // instead is to generate types as vars, avoid cycles which are problematic,
  // and once that's done, we can switch this to a Set.
  // TODO(fxb/8008): Use this set to determine whether this value is unknown.
  // (Alternative design would be to have a boolean field isUnknown.)
  final Map<int, Null> values;
  final EnumFactory<T> ctor;

  @override
  int get inlineSize => type.inlineSize;

  @override
  void encode(Encoder encoder, T value, int offset, int depth) {
    type.encode(encoder, value.$value, offset, depth);
  }

  @override
  T decode(Decoder decoder, int offset, int depth) {
    return ctor(type.decode(decoder, offset, depth));
  }
}

class BitsType<T extends Bits> extends SimpleFidlType<T> {
  const BitsType({
    required this.type,
    required this.ctor,
  }) : super(inlineSize: 0); // Note: inlineSize is calculated below

  final FidlType<int, dynamic> type;
  final BitsFactory<T> ctor;

  @override
  int get inlineSize => type.inlineSize;

  @override
  void encode(Encoder encoder, T value, int offset, int depth) {
    type.encode(encoder, value.$value, offset, depth);
  }

  @override
  T decode(Decoder decoder, int offset, int depth) {
    return ctor(type.decode(decoder, offset, depth));
  }
}

class MethodType {
  const MethodType({
    required this.request,
    required this.response,
    required this.name,
    required this.requestInlineSize,
    required this.responseInlineSize,
  });

  final List<MemberType>? request;
  final List<MemberType>? response;
  final String name;
  final int requestInlineSize;
  final int responseInlineSize;

  int encodingRequestInlineSize() {
    return requestInlineSize;
  }

  int encodingResponseInlineSize() {
    return responseInlineSize;
  }

  int decodeRequestInlineSize() {
    return requestInlineSize;
  }

  int decodeResponseInlineSize() {
    return responseInlineSize;
  }
}

void _encodeVector<T, I extends Iterable<T>>(
    Encoder encoder,
    FidlType<T, I> element,
    Iterable<T> value,
    int offset,
    int depth,
    int? maybeElementCount) {
  final int count = value.length;
  _throwIfExceedsLimit(count, maybeElementCount);
  encoder
    ..encodeUint64(count, offset) // count
    ..encodeUint64(kAllocPresent, offset + 8); // data
  int childOffset = encoder.alloc(count * element.encodingInlineSize(), depth);
  element.encodeArray(encoder, value, childOffset, depth + 1);
}

I? _decodeVector<T, I extends Iterable<T>>(Decoder decoder,
    FidlType<T, I> element, int offset, int depth, int? maybeElementCount) {
  final int count = decoder.decodeUint64(offset);
  final int data = decoder.decodeUint64(offset + 8);
  _throwIfExceedsLimit(count, maybeElementCount);
  if (data == kAllocAbsent) {
    return null;
  }
  final int base =
      decoder.claimMemory(count * element.decodingInlineSize(), depth);
  I? out = element.decodeArray(decoder, count, base, depth + 1);
  return out;
}

class VectorType<T, I extends Iterable<T>> extends SimpleFidlType<I> {
  const VectorType({
    required this.element,
    required this.maybeElementCount,
  }) : super(inlineSize: 16);

  final FidlType<T, I> element;
  final int? maybeElementCount;

  @override
  void encode(Encoder encoder, Iterable<T> value, int offset, int depth) =>
      _encodeVector(encoder, element, value, offset, depth, maybeElementCount);

  @override
  I decode(Decoder decoder, int offset, int depth) {
    I? value =
        _decodeVector<T, I>(decoder, element, offset, depth, maybeElementCount);
    if (value == null) {
      throw _notNullable;
    }
    return value;
  }
}

class NullableVectorType<T, I extends Iterable<T>> extends SimpleFidlType<I?> {
  const NullableVectorType({
    required this.element,
    required this.maybeElementCount,
  }) : super(inlineSize: 16);

  final FidlType<T, I> element;
  final int? maybeElementCount;

  @override
  void encode(Encoder encoder, Iterable<T>? value, int offset, int depth) {
    if (value == null) {
      _encodeNullVector(encoder, offset);
    } else {
      _encodeVector(encoder, element, value, offset, depth, maybeElementCount);
    }
  }

  @override
  I? decode(Decoder decoder, int offset, int depth) =>
      _decodeVector(decoder, element, offset, depth, maybeElementCount);
}

class ArrayType<T, I extends Iterable<T>> extends SimpleFidlType<I> {
  const ArrayType({
    required this.element,
    required this.elementCount,
  }) : super(inlineSize: 0); // Note: inlineSize is calculated below

  final FidlType<T, I> element;
  final int elementCount;

  @override
  int get inlineSize => elementCount * element.inlineSize;

  @override
  void encode(Encoder encoder, Iterable<T> value, int offset, int depth) {
    _throwIfCountMismatch(value.length, elementCount);
    element.encodeArray(encoder, value, offset, depth);
  }

  @override
  I decode(Decoder decoder, int offset, int depth) =>
      element.decodeArray(decoder, elementCount, offset, depth);
}
