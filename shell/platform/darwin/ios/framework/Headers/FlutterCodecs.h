// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERCODECS_H_
#define FLUTTER_FLUTTERCODECS_H_

#import <Foundation/Foundation.h>
#include "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

FLUTTER_EXPORT
@protocol FlutterMessageCodec
+ (instancetype)sharedInstance;
- (NSData* _Nullable)encode:(id _Nullable)message;
- (id _Nullable)decode:(NSData* _Nullable)message;
@end

FLUTTER_EXPORT
@interface FlutterBinaryCodec : NSObject<FlutterMessageCodec>
@end

FLUTTER_EXPORT
@interface FlutterStringCodec : NSObject<FlutterMessageCodec>
@end

FLUTTER_EXPORT
@interface FlutterJSONMessageCodec : NSObject<FlutterMessageCodec>
@end

FLUTTER_EXPORT
@interface FlutterStandardMessageCodec : NSObject<FlutterMessageCodec>
@end

FLUTTER_EXPORT
@interface FlutterMethodCall : NSObject
+ (instancetype)methodCallWithMethodName:(NSString*)method
                               arguments:(id _Nullable)arguments;
@property(readonly, nonatomic) NSString* method;
@property(readonly, nonatomic, nullable) id arguments;
@end

FLUTTER_EXPORT
@interface FlutterError : NSObject
+ (instancetype)errorWithCode:(NSString*)code
                      message:(NSString* _Nullable)message
                      details:(id _Nullable)details;
@property(readonly, nonatomic) NSString* code;
@property(readonly, nonatomic, nullable) NSString* message;
@property(readonly, nonatomic, nullable) id details;
@end

typedef NS_ENUM(NSInteger, FlutterStandardDataType) {
  FlutterStandardDataTypeUInt8,
  FlutterStandardDataTypeInt32,
  FlutterStandardDataTypeInt64,
  FlutterStandardDataTypeFloat64,
};

FLUTTER_EXPORT
@interface FlutterStandardTypedData : NSObject
+ (instancetype)typedDataWithBytes:(NSData*)data;
+ (instancetype)typedDataWithInt32:(NSData*)data;
+ (instancetype)typedDataWithInt64:(NSData*)data;
+ (instancetype)typedDataWithFloat64:(NSData*)data;
@property(readonly, nonatomic) NSData* data;
@property(readonly, nonatomic) FlutterStandardDataType type;
@property(readonly, nonatomic) UInt32 elementCount;
@property(readonly, nonatomic) UInt8 elementSize;
@end

FLUTTER_EXPORT
@interface FlutterStandardBigInteger : NSObject
+ (instancetype)bigIntegerWithHex:(NSString*)hex;
@property(readonly, nonatomic) NSString* hex;
@end

FLUTTER_EXPORT
@protocol FlutterMethodCodec
+ (instancetype)sharedInstance;
- (NSData*)encodeMethodCall:(FlutterMethodCall*)methodCall;
- (FlutterMethodCall*)decodeMethodCall:(NSData*)methodCall;
- (NSData*)encodeSuccessEnvelope:(id _Nullable)result;
- (NSData*)encodeErrorEnvelope:(FlutterError*)error;
- (id _Nullable)decodeEnvelope:(NSData*)envelope;
@end

FLUTTER_EXPORT
@interface FlutterJSONMethodCodec : NSObject<FlutterMethodCodec>
@end

FLUTTER_EXPORT
@interface FlutterStandardMethodCodec : NSObject<FlutterMethodCodec>
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_FLUTTERCODECS_H_
