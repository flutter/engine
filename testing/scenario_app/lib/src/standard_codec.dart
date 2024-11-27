// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:math' as math;
import 'dart:typed_data';

const int __writeBufferStartCapacity = 64;

/// This is mirroring the standard codec from framework
class StandardMessageCodec {
  /// Creates a [MessageCodec] using the Flutter standard binary encoding.
  const StandardMessageCodec();
  static const int _valueNull = 0;
  static const int _valueTrue = 1;
  static const int _valueFalse = 2;
  static const int _valueInt32 = 3;
  static const int _valueInt64 = 4;
  // Unused
  // static const int _valueLargeInt = 5;
  static const int _valueFloat64 = 6;
  static const int _valueString = 7;
  static const int _valueUint8List = 8;
  static const int _valueInt32List = 9;
  static const int _valueInt64List = 10;
  static const int _valueFloat64List = 11;
  static const int _valueList = 12;
  static const int _valueMap = 13;
  static const int _valueFloat32List = 14;

  /// Encode the message.
  ByteData? encodeMessage(Object? message) {
    if (message == null) {
      return null;
    }
    final _WriteBuffer buffer = _WriteBuffer(startCapacity: __writeBufferStartCapacity);
    _writeValue(buffer, message);
    return buffer.done();
  }

  void _writeValue(_WriteBuffer buffer, Object? value) {
    if (value == null) {
      buffer.putUint8(_valueNull);
    } else if (value is bool) {
      buffer.putUint8(value ? _valueTrue : _valueFalse);
    } else if (value is double) {  // Double precedes int because in JS everything is a double.
                                   // Therefore in JS, both `is int` and `is double` always
                                   // return `true`. If we check int first, we'll end up treating
                                   // all numbers as ints and attempt the int32/int64 conversion,
                                   // which is wrong. This precedence rule is irrelevant when
                                   // decoding because we use tags to detect the type of value.
      buffer.putUint8(_valueFloat64);
      buffer.putFloat64(value);
    } else if (value is int) { // ignore: avoid_double_and_int_checks, JS code always goes through the `double` path above
      if (-0x7fffffff - 1 <= value && value <= 0x7fffffff) {
        buffer.putUint8(_valueInt32);
        buffer.putInt32(value);
      } else {
        buffer.putUint8(_valueInt64);
        buffer.putInt64(value);
      }
    } else if (value is String) {
      buffer.putUint8(_valueString);
      final Uint8List asciiBytes = Uint8List(value.length);
      Uint8List? utf8Bytes;
      int utf8Offset = 0;
      // Only do utf8 encoding if we encounter non-ascii characters.
      for (int i = 0; i < value.length; i += 1) {
        final int char = value.codeUnitAt(i);
        if (char <= 0x7f) {
          asciiBytes[i] = char;
        } else {
          utf8Bytes = utf8.encode(value.substring(i));
          utf8Offset = i;
          break;
        }
      }
      if (utf8Bytes != null) {
        _writeSize(buffer, utf8Offset + utf8Bytes.length);
        buffer.putUint8List(Uint8List.sublistView(asciiBytes, 0, utf8Offset));
        buffer.putUint8List(utf8Bytes);
      } else {
        _writeSize(buffer, asciiBytes.length);
        buffer.putUint8List(asciiBytes);
      }
    } else if (value is Uint8List) {
      buffer.putUint8(_valueUint8List);
      _writeSize(buffer, value.length);
      buffer.putUint8List(value);
    } else if (value is Int32List) {
      buffer.putUint8(_valueInt32List);
      _writeSize(buffer, value.length);
      buffer.putInt32List(value);
    } else if (value is Int64List) {
      buffer.putUint8(_valueInt64List);
      _writeSize(buffer, value.length);
      buffer.putInt64List(value);
    } else if (value is Float32List) {
      buffer.putUint8(_valueFloat32List);
      _writeSize(buffer, value.length);
      buffer.putFloat32List(value);
    } else if (value is Float64List) {
      buffer.putUint8(_valueFloat64List);
      _writeSize(buffer, value.length);
      buffer.putFloat64List(value);
    } else if (value is List) {
      buffer.putUint8(_valueList);
      _writeSize(buffer, value.length);
      for (final Object? item in value) {
        _writeValue(buffer, item);
      }
    } else if (value is Map) {
      buffer.putUint8(_valueMap);
      _writeSize(buffer, value.length);
      value.forEach((Object? key, Object? value) {
        _writeValue(buffer, key);
        _writeValue(buffer, value);
      });
    } else {
      throw ArgumentError.value(value);
    }
  }

  void _writeSize(_WriteBuffer buffer, int value) {
    assert(0 <= value && value <= 0xffffffff);
    if (value < 254) {
      buffer.putUint8(value);
    } else if (value <= 0xffff) {
      buffer.putUint8(254);
      buffer.putUint16(value);
    } else {
      buffer.putUint8(255);
      buffer.putUint32(value);
    }
  }
}


class _WriteBuffer {
  factory _WriteBuffer({int startCapacity = 8}) {
    assert(startCapacity > 0);
    final ByteData eightBytes = ByteData(8);
    final Uint8List eightBytesAsList = eightBytes.buffer.asUint8List();
    return _WriteBuffer._(Uint8List(startCapacity), eightBytes, eightBytesAsList);
  }

  _WriteBuffer._(this._buffer, this._eightBytes, this._eightBytesAsList);

  Uint8List _buffer;
  int _currentSize = 0;
  bool _isDone = false;
  final ByteData _eightBytes;
  final Uint8List _eightBytesAsList;
  static final Uint8List _zeroBuffer = Uint8List(8);

  void _add(int byte) {
    if (_currentSize == _buffer.length) {
      _resize();
    }
    _buffer[_currentSize] = byte;
    _currentSize += 1;
  }

  void _append(Uint8List other) {
    final int newSize = _currentSize + other.length;
    if (newSize >= _buffer.length) {
      _resize(newSize);
    }
    _buffer.setRange(_currentSize, newSize, other);
    _currentSize += other.length;
  }

  void _addAll(Uint8List data, [int start = 0, int? end]) {
    final int newEnd = end ?? _eightBytesAsList.length;
    final int newSize = _currentSize + (newEnd - start);
    if (newSize >= _buffer.length) {
      _resize(newSize);
    }
    _buffer.setRange(_currentSize, newSize, data);
    _currentSize = newSize;
  }

  void _resize([int? requiredLength]) {
    final int doubleLength = _buffer.length * 2;
    final int newLength = math.max(requiredLength ?? 0, doubleLength);
    final Uint8List newBuffer = Uint8List(newLength);
    newBuffer.setRange(0, _buffer.length, _buffer);
    _buffer = newBuffer;
  }

  /// Write a Uint8 into the buffer.
  void putUint8(int byte) {
    assert(!_isDone);
    _add(byte);
  }

  /// Write a Uint16 into the buffer.
  void putUint16(int value, {Endian? endian}) {
    assert(!_isDone);
    _eightBytes.setUint16(0, value, endian ?? Endian.host);
    _addAll(_eightBytesAsList, 0, 2);
  }

  /// Write a Uint32 into the buffer.
  void putUint32(int value, {Endian? endian}) {
    assert(!_isDone);
    _eightBytes.setUint32(0, value, endian ?? Endian.host);
    _addAll(_eightBytesAsList, 0, 4);
  }

  /// Write an Int32 into the buffer.
  void putInt32(int value, {Endian? endian}) {
    assert(!_isDone);
    _eightBytes.setInt32(0, value, endian ?? Endian.host);
    _addAll(_eightBytesAsList, 0, 4);
  }

  /// Write an Int64 into the buffer.
  void putInt64(int value, {Endian? endian}) {
    assert(!_isDone);
    _eightBytes.setInt64(0, value, endian ?? Endian.host);
    _addAll(_eightBytesAsList, 0, 8);
  }

  /// Write an Float64 into the buffer.
  void putFloat64(double value, {Endian? endian}) {
    assert(!_isDone);
    _alignTo(8);
    _eightBytes.setFloat64(0, value, endian ?? Endian.host);
    _addAll(_eightBytesAsList);
  }

  /// Write all the values from a [Uint8List] into the buffer.
  void putUint8List(Uint8List list) {
    assert(!_isDone);
    _append(list);
  }

  /// Write all the values from an [Int32List] into the buffer.
  void putInt32List(Int32List list) {
    assert(!_isDone);
    _alignTo(4);
    _append(list.buffer.asUint8List(list.offsetInBytes, 4 * list.length));
  }

  /// Write all the values from an [Int64List] into the buffer.
  void putInt64List(Int64List list) {
    assert(!_isDone);
    _alignTo(8);
    _append(list.buffer.asUint8List(list.offsetInBytes, 8 * list.length));
  }

  /// Write all the values from a [Float32List] into the buffer.
  void putFloat32List(Float32List list) {
    assert(!_isDone);
    _alignTo(4);
    _append(list.buffer.asUint8List(list.offsetInBytes, 4 * list.length));
  }

  /// Write all the values from a [Float64List] into the buffer.
  void putFloat64List(Float64List list) {
    assert(!_isDone);
    _alignTo(8);
    _append(list.buffer.asUint8List(list.offsetInBytes, 8 * list.length));
  }

  void _alignTo(int alignment) {
    assert(!_isDone);
    final int mod = _currentSize % alignment;
    if (mod != 0) {
      _addAll(_zeroBuffer, 0, alignment - mod);
    }
  }

  /// Finalize and return the written [ByteData].
  ByteData done() {
    if (_isDone) {
      throw StateError('done() must not be called more than once on the same $runtimeType.');
    }
    final ByteData result = _buffer.buffer.asByteData(0, _currentSize);
    _buffer = Uint8List(0);
    _isDone = true;
    return result;
  }
}
