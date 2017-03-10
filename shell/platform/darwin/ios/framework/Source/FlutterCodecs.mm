// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterCodecs.h"
#include "flutter/shell/platform/darwin/common/string_conversions.h"

@implementation FlutterBinaryCodec
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    _sharedInstance = [FlutterBinaryCodec new];
  }
  return _sharedInstance;
}

- (NSData*)encode:(NSData*)message {
  return message;
}

- (NSData*)decode:(NSData*)message {
  return message;
}
@end

@implementation FlutterStringCodec
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    _sharedInstance = [FlutterStringCodec new];
  }
  return _sharedInstance;
}

- (NSData*)encode:(NSString*)message {
  return shell::GetNSDataFromNSString(message);
}

- (NSString*)decode:(NSData*)message {
  return shell::GetNSStringFromNSData(message);
}
@end

@implementation FlutterJSONMessageCodec
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    _sharedInstance = [FlutterJSONMessageCodec new];
  }
  return _sharedInstance;
}

- (NSData*)encode:(id)message {
  return [NSJSONSerialization dataWithJSONObject:message options:0 error:nil];
}

- (id)decode:(NSData*)message {
  return [NSJSONSerialization JSONObjectWithData:message options:0 error:nil];
}
@end

@implementation FlutterJSONMethodCodec
+ (instancetype)sharedInstance {
  static id _sharedInstance = nil;
  if (!_sharedInstance) {
    _sharedInstance = [FlutterJSONMethodCodec new];
  }
  return _sharedInstance;
}

- (NSData*)encodeSuccessEnvelope:(id)result {
  return [[FlutterJSONMessageCodec sharedInstance] encode:@[ result ]];
}

- (NSData*)encodeErrorEnvelope:(FlutterError*)error {
  return [[FlutterJSONMessageCodec sharedInstance]
      encode:@[ error.code, error.message, error.details ]];
}

- (FlutterMethodCall*)decodeMethodCall:(NSData*)message {
  NSArray* call = [[FlutterJSONMessageCodec sharedInstance] decode:message];
  return [FlutterMethodCall methodCallWithMethodName:call[0] arguments:call[1]];
}
@end
