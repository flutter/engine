// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

// TODO(fxbug.dev/8076) Generate these values.
enum FidlErrorCode {
  unknown,
  fidlExceededMaxOutOfLineDepth,
  fidlInvalidPresenceIndicator,
  fidlInvalidNumBytesInEnvelope,
  fidlInvalidNumHandlesInEnvelope,
  fidlTooManyHandles,
  fidlTooFewHandles,
  fidlStringTooLong,
  fidlNonNullableTypeWithNullValue,
  fidlStrictXUnionUnknownField,
  fidlUnknownMagic,
  fidlInvalidBit,
  fidlInvalidEnumValue,
  fidlIntOutOfRange,
  fidlNonEmptyStringWithNullBody,
  fidlNonResourceHandle,
}

class FidlError implements Exception {
  // TODO(fxbug.dev/7865) Make code a required parameter.
  const FidlError(this.message, [this.code = FidlErrorCode.unknown]);

  final String message;
  final FidlErrorCode code;

  @override
  String toString() => 'FidlError: $message';
}

class FidlRangeCheckError implements FidlError {
  FidlRangeCheckError(this.value, this.min, this.max);

  final int value;
  final int min;
  final int max;
  @override
  String get message => 'FidlRangeCheckError: $value < $min or $value > $max';
  @override
  FidlErrorCode get code => FidlErrorCode.fidlIntOutOfRange;
}

class MethodException<T> implements Exception {
  MethodException(this.value);

  final T value;

  @override
  String toString() => 'MethodException: $value';
}
