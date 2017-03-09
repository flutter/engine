// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERCODECS_H_
#define FLUTTER_FLUTTERCODECS_H_

#import <Foundation/Foundation.h>
#include "FlutterMacros.h"

FLUTTER_EXPORT
@protocol FlutterMessageCodec
+ (instancetype)shared;
- (NSData*)encode:(id)message;
- (id)decode:(NSData*)message;
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
+ (instancetype) withMethod:(NSString*)method andArguments:(id)arguments;
@property(readonly) NSString* method;
@property(readonly) id arguments;
@end

FLUTTER_EXPORT
@interface FlutterError : NSObject
+ (instancetype) withCode:(NSString*)code message:(NSString*)message details:(id)details;
@property(readonly) NSString* code;
@property(readonly) NSString* message;
@property(readonly) id details;
@end

FLUTTER_EXPORT
@interface FlutterStandardTypedData : NSObject
+ (instancetype) withBytes:(NSData*)data;
+ (instancetype) withInt32:(NSData*)data;
+ (instancetype) withInt64:(NSData*)data;
+ (instancetype) withFloat64:(NSData*)data;
@property(readonly) NSData* data;
@property(readonly) UInt32 length;
@property(readonly) UInt8 elementSize;
@property(readonly) UInt8 type;
@end

FLUTTER_EXPORT
@interface FlutterStandardBigInteger : NSObject
+ (instancetype) withHex:(NSString*)hex;
@property(readonly) NSString* hex;
@end

FLUTTER_EXPORT
@protocol FlutterMethodCodec
+ (instancetype) shared;
- (FlutterMethodCall*) decodeMethodCall:(NSData*)message;
- (NSData*) encodeSuccessEnvelope:(id)result;
- (NSData*) encodeErrorEnvelope:(FlutterError*)error;
@end

FLUTTER_EXPORT
@interface FlutterJSONMethodCodec : NSObject<FlutterMethodCodec>
@end

FLUTTER_EXPORT
@interface FlutterStandardMethodCodec : NSObject<FlutterMethodCodec>
@end

#endif  // FLUTTER_FLUTTERCODECS_H_
