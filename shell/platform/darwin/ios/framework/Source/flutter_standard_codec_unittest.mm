// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterCodecs.h"
#include "gtest/gtest.h"

TEST(FlutterStandardCodec, CanEncodeAndDecodeNil) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:nil];
  id decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded == nil);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeNSNull) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:[NSNull null]];
  id decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded == nil);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeInt32) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@-78];
  NSNumber* decoded = [codec decode:encoded];
  ASSERT_TRUE([@-78 isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeInt64) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@78000000001];
  NSNumber* decoded = [codec decode:encoded];
  ASSERT_TRUE([@78000000001 isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeFloat64) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@3.14];
  NSNumber* decoded = [codec decode:encoded];
  ASSERT_TRUE([@3.14 isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeString) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@"hello world"];
  NSString* decoded = [codec decode:encoded];
  ASSERT_TRUE([@"hello world" isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeStringWithNonAsciiCodePoint) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@"hello \u263A world"];
  NSString* decoded = [codec decode:encoded];
  ASSERT_TRUE([@"hello \u263A world" isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeStringWithNonBMPCodePoint) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:@"hello \U0001F602 world"];
  NSString* decoded = [codec decode:encoded];
  ASSERT_TRUE([@"hello \U0001F602 world" isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeBigInteger) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded =
      [codec encode:[FlutterStandardBigInteger
                        bigIntegerWithHex:@"-abcdef120902390239021321abfdec"]];
  FlutterStandardBigInteger* decoded = [codec decode:encoded];
  ASSERT_TRUE([@"-abcdef120902390239021321abfdec" isEqualTo:decoded.hex]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeArray) {
  NSArray* value =
      @[ [NSNull null], @"hello", @3.14, @47,
         @{ @42 : @"nested" } ];
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  NSArray* decoded = [codec decode:encoded];
  ASSERT_TRUE([value isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeDictionary) {
  NSDictionary* value = @{
    @"a" : @3.14,
    @"b" : @47,
    [NSNull null] : [NSNull null],
    @3.14 : @[ @"nested" ]
  };
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  NSDictionary* decoded = [codec decode:encoded];
  ASSERT_TRUE([value isEqualTo:decoded]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeByteArray) {
  char bytes[4] = {0xBA, 0x5E, 0xBA, 0x11};
  NSData* data = [NSData dataWithBytes:bytes length:4];
  FlutterStandardTypedData* value =
      [FlutterStandardTypedData typedDataWithBytes:data];
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  FlutterStandardTypedData* decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded.type == FlutterStandardDataTypeUInt8);
  ASSERT_TRUE(decoded.elementCount == 4);
  ASSERT_TRUE(decoded.elementSize == 1);
  ASSERT_TRUE([data isEqualTo:decoded.data]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeInt32Array) {
  char bytes[8] = {0xBA, 0x5E, 0xBA, 0x11, 0xff, 0xff, 0xff, 0xff};
  NSData* data = [NSData dataWithBytes:bytes length:8];
  FlutterStandardTypedData* value =
      [FlutterStandardTypedData typedDataWithInt32:data];
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  FlutterStandardTypedData* decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded.type == FlutterStandardDataTypeInt32);
  ASSERT_TRUE(decoded.elementCount == 2);
  ASSERT_TRUE(decoded.elementSize == 4);
  ASSERT_TRUE([data isEqualTo:decoded.data]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeInt64Array) {
  char bytes[8] = {0xBA, 0x5E, 0xBA, 0x11, 0xff, 0xff, 0xff, 0xff};
  NSData* data = [NSData dataWithBytes:bytes length:8];
  FlutterStandardTypedData* value =
      [FlutterStandardTypedData typedDataWithInt64:data];
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  FlutterStandardTypedData* decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded.type == FlutterStandardDataTypeInt64);
  ASSERT_TRUE(decoded.elementCount == 1);
  ASSERT_TRUE(decoded.elementSize == 8);
  ASSERT_TRUE([data isEqualTo:decoded.data]);
}

TEST(FlutterStandardCodec, CanEncodeAndDecodeFloat64Array) {
  char bytes[16] = {0xBA, 0x5E, 0xBA, 0x11, 0xff, 0xff, 0xff, 0xff,
                    0xBA, 0x5E, 0xBA, 0x11, 0xff, 0xff, 0xff, 0xff};
  NSData* data = [NSData dataWithBytes:bytes length:16];
  FlutterStandardTypedData* value =
      [FlutterStandardTypedData typedDataWithFloat64:data];
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  NSData* encoded = [codec encode:value];
  FlutterStandardTypedData* decoded = [codec decode:encoded];
  ASSERT_TRUE(decoded.type == FlutterStandardDataTypeFloat64);
  ASSERT_TRUE(decoded.elementCount == 2);
  ASSERT_TRUE(decoded.elementSize == 8);
  ASSERT_TRUE([data isEqualTo:decoded.data]);
}

TEST(FlutterStandardCodec, AgreesWithJSONCodecOnBooleans) {
  FlutterStandardMessageCodec* stdCodec =
      [FlutterStandardMessageCodec sharedInstance];
  FlutterJSONMessageCodec* jsonCodec = [FlutterJSONMessageCodec sharedInstance];
  NSArray* values = @[ @(true), @(false), @YES, @NO ];
  NSData* stdEncoded = [stdCodec encode:values];
  NSArray* stdDecoded = [stdCodec decode:stdEncoded];
  NSData* jsonEncoded = [jsonCodec encode:values];
  NSArray* jsonDecoded = [jsonCodec decode:jsonEncoded];
  ASSERT_TRUE([stdDecoded isEqualTo:jsonDecoded]);
  ASSERT_TRUE([stdDecoded isEqualTo:values]);
}

TEST(FlutterStandardCodec, AgreesWithJSONCodecOnSmallIntegers) {
  FlutterStandardMessageCodec* stdCodec =
      [FlutterStandardMessageCodec sharedInstance];
  FlutterJSONMessageCodec* jsonCodec = [FlutterJSONMessageCodec sharedInstance];
  UInt8 u8 = 0xff;
  UInt16 u16 = 0xffff;
  UInt32 u32 = 0xffffffff;
  SInt8 s8 = 0xff;
  SInt16 s16 = 0xffff;
  SInt32 s32 = 0xffffffff;
  SInt64 s64 = 0xffffffffffffffff;
  NSArray* values = @[ @(u8), @(u16), @(u32), @(s8), @(s16), @(s32), @(s64) ];
  NSData* stdEncoded = [stdCodec encode:values];
  NSArray* stdDecoded = [stdCodec decode:stdEncoded];
  NSData* jsonEncoded = [jsonCodec encode:values];
  NSArray* jsonDecoded = [jsonCodec decode:jsonEncoded];
  ASSERT_TRUE([stdDecoded isEqualTo:jsonDecoded]);
  ASSERT_TRUE([stdDecoded isEqualTo:values]);
}

TEST(FlutterStandardCodec, UsesHexOnLargeIntegers) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  UInt64 u64 = 0xffffffffffffffff;
  NSData* encoded = [codec encode:@(u64)];
  FlutterStandardBigInteger* decoded = [codec decode:encoded];
  ASSERT_TRUE([decoded.hex isEqualTo:@"ffffffffffffffff"]);
}

TEST(FlutterStandardCodec, Uses64BitFloats) {
  FlutterStandardMessageCodec* codec =
      [FlutterStandardMessageCodec sharedInstance];
  float f32 = 0.1f;
  double f64 = 0.1;
  double f32converted = 0.1f;
  NSArray* values = @[ @(f32), @(f64) ];
  NSArray* decodedValues = @[ @(f32converted), @(f64) ];
  NSData* encoded = [codec encode:values];
  NSArray* decoded = [codec decode:encoded];
  ASSERT_TRUE([decoded isEqualTo:decodedValues]);
}

TEST(FlutterStandardCodec, HandlesMethodCallsWithNilArguments) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  FlutterMethodCall* call =
      [FlutterMethodCall methodCallWithMethodName:@"hello" arguments:nil];
  NSData* encoded = [codec encodeMethodCall:call];
  FlutterMethodCall* decoded = [codec decodeMethodCall:encoded];
  ASSERT_TRUE([decoded.method isEqualTo:call.method]);
  ASSERT_EQ(decoded.arguments, nil);
}

TEST(FlutterStandardCodec, HandlesMethodCallsWithSingleArgument) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  FlutterMethodCall* call =
      [FlutterMethodCall methodCallWithMethodName:@"hello" arguments:@42];
  NSData* encoded = [codec encodeMethodCall:call];
  FlutterMethodCall* decoded = [codec decodeMethodCall:encoded];
  ASSERT_TRUE([decoded.method isEqualTo:call.method]);
  ASSERT_EQ(decoded.arguments, @42);
}

TEST(FlutterStandardCodec, HandlesMethodCallsWithArgumentList) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  NSArray* arguments = @[ @42, @"world" ];
  FlutterMethodCall* call =
      [FlutterMethodCall methodCallWithMethodName:@"hello" arguments:arguments];
  NSData* encoded = [codec encodeMethodCall:call];
  FlutterMethodCall* decoded = [codec decodeMethodCall:encoded];
  ASSERT_TRUE([decoded.method isEqualTo:call.method]);
  ASSERT_TRUE([decoded.arguments isEqualTo:arguments]);
}

TEST(FlutterStandardCodec, HandlesSuccessEnvelopesWithNilResult) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  NSData* encoded = [codec encodeSuccessEnvelope:nil];
  FlutterError* error = nil;
  id decoded = [codec decodeEnvelope:encoded error:&error];
  ASSERT_EQ(error, nil);
  ASSERT_EQ(decoded, nil);
}

TEST(FlutterStandardCodec, HandlesSuccessEnvelopesWithSingleResult) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  NSData* encoded = [codec encodeSuccessEnvelope:@42];
  FlutterError* decodedError = nil;
  id decodedResult = [codec decodeEnvelope:encoded error:&decodedError];
  ASSERT_EQ(decodedError, nil);
  ASSERT_EQ(decodedResult, @42);
}

TEST(FlutterStandardCodec, HandlesSuccessEnvelopesWithResultMap) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  NSDictionary* result = @{ @"a" : @42, @42 : @"a" };
  NSData* encoded = [codec encodeSuccessEnvelope:result];
  FlutterError* decodedError = nil;
  id decodedResult = [codec decodeEnvelope:encoded error:&decodedError];
  ASSERT_TRUE([decodedResult isEqualTo:result]);
}

TEST(FlutterStandardCodec, HandlesErrorEnvelopes) {
  FlutterStandardMethodCodec* codec =
      [FlutterStandardMethodCodec sharedInstance];
  NSDictionary* details = @{ @"a" : @42, @42 : @"a" };
  FlutterError* error = [FlutterError errorWithCode:@"errorCode"
                                            message:@"something failed"
                                            details:details];
  NSData* encoded = [codec encodeErrorEnvelope:error];
  FlutterError* decodedError = nil;
  id decodedResult = [codec decodeEnvelope:encoded error:&decodedError];
  ASSERT_EQ(decodedResult, nil);
  ASSERT_TRUE([decodedError.code isEqualTo:error.code]);
  ASSERT_TRUE([decodedError.message isEqualTo:error.message]);
  ASSERT_TRUE([decodedError.details isEqualTo:error.details]);
}
