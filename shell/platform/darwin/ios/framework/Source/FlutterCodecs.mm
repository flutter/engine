// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/common/string_conversions.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterCodecs.h"

@implementation FlutterBinaryCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterBinaryCodec new];
  }
  return _shared;
}

- (NSData*) encode:(NSData*)message {
  return message;
}

- (NSData*) decode:(NSData*)message {
  return message;
}
@end

@implementation FlutterStringCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterStringCodec new];
  }
  return _shared;
}

- (NSData*) encode:(NSString*)message {
  return shell::GetNSDataFromNSString(message);
}

- (NSString*) decode:(NSData*)message {
  return shell::GetNSStringFromNSData(message);
}
@end

@implementation FlutterJSONMessageCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterJSONMessageCodec new];
  }
  return _shared;
}

- (NSData*) encode:(id)message {
  return [NSJSONSerialization dataWithJSONObject:message options:0 error:nil];
}

- (id) decode:(NSData*)message {
  return [NSJSONSerialization JSONObjectWithData:message options:0 error:nil];
}
@end

@implementation FlutterJSONMethodCodec
+ (instancetype) shared {
  static id _shared = nil;
  if (!_shared) {
     _shared = [FlutterJSONMethodCodec new];
  }
  return _shared;
}

- (NSData*) encodeSuccessEnvelope:(id)result {
  return [[FlutterJSONMessageCodec shared] encode:@[result]];
}

- (NSData*) encodeErrorEnvelope:(FlutterError*)error {
  return [[FlutterJSONMessageCodec shared] encode:@[error.code, error.message, error.details]];
}

- (FlutterMethodCall*) decodeMethodCall:(NSData*)message {
  NSArray* call = [[FlutterJSONMessageCodec shared] decode:message];
  return [FlutterMethodCall withMethod:call[0] andArguments:call[1]];
}
@end
