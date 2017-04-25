// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterCodecs.h"

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
  if (message == nil)
    return nil;
  const char* utf8 = message.UTF8String;
  return [NSData dataWithBytes:utf8 length:strlen(utf8)];
}

- (NSString*)decode:(NSData*)message {
  if (message == nil)
    return nil;
  return [[[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding] autorelease];
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
  if (message == nil)
    return nil;
  NSData* encoding;
  if ([message isKindOfClass:[NSArray class]] || [message isKindOfClass:[NSDictionary class]]) {
    encoding = [NSJSONSerialization dataWithJSONObject:message options:0 error:nil];
  } else {
    // NSJSONSerialization does not support top-level simple values.
    // We encode as singleton array, then extract the relevant bytes.
    encoding = [NSJSONSerialization dataWithJSONObject:@[message] options:0 error:nil];
    if (encoding) {
      encoding = [encoding subdataWithRange:NSMakeRange(1, encoding.length - 2)];
    }
  }
  NSAssert(encoding, @"Invalid JSON message, encoding failed");
  return encoding;
}

- (id)decode:(NSData*)message {
  if (message == nil)
    return nil;
  BOOL isSimpleValue = NO;
  id decoded = nil;
  if (0 < message.length) {
    UInt8 first;
    [message getBytes:&first length:1];
    isSimpleValue = first != '{' && first != '[';
    if (isSimpleValue) {
      UInt8 begin = '[';
      UInt8 end = ']';
      NSMutableData* expandedMessage = [NSMutableData dataWithLength:message.length + 2];
      [expandedMessage replaceBytesInRange:NSMakeRange(0, 1) withBytes:&begin];
      [expandedMessage replaceBytesInRange:NSMakeRange(1, message.length) withBytes:message.bytes];
      [expandedMessage replaceBytesInRange:NSMakeRange(message.length + 1, 1) withBytes:&end];
      message = expandedMessage;
    }
    decoded = [NSJSONSerialization JSONObjectWithData:message options:0 error:nil];
  }
  NSAssert(decoded, @"Invalid JSON message, decoding failed");
  return isSimpleValue ? ((NSArray*) decoded)[0] : decoded;
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

- (NSData*)encodeMethodCall:(FlutterMethodCall*)call {
  return [[FlutterJSONMessageCodec sharedInstance] encode:@{
    @"method" : call.method,
    @"args" : (call.arguments == nil ? [NSNull null] : call.arguments),
  }];
}

- (NSData*)encodeSuccessEnvelope:(id)result {
  return
      [[FlutterJSONMessageCodec sharedInstance] encode:@[ result == nil ? [NSNull null] : result ]];
}

- (NSData*)encodeErrorEnvelope:(FlutterError*)error {
  return [[FlutterJSONMessageCodec sharedInstance] encode:@[
    error.code,
    error.message == nil ? [NSNull null] : error.message,
    error.details == nil ? [NSNull null] : error.details,
  ]];
}

- (FlutterMethodCall*)decodeMethodCall:(NSData*)message {
  NSDictionary* dictionary = [[FlutterJSONMessageCodec sharedInstance] decode:message];
  id method = dictionary[@"method"];
  id arguments = dictionary[@"args"];
  NSAssert([method isKindOfClass:[NSString class]], @"Invalid JSON method call");
  return [FlutterMethodCall methodCallWithMethodName:method arguments:arguments];
}

- (id)decodeEnvelope:(NSData*)envelope {
  NSArray* array = [[FlutterJSONMessageCodec sharedInstance] decode:envelope];
  if (array.count == 1)
    return array[0];
  NSAssert(array.count == 3, @"Invalid JSON envelope");
  NSAssert([array[0] isKindOfClass:[NSString class]], @"Invalid JSON envelope");
  NSAssert(array[1] == nil || [array[1] isKindOfClass:[NSString class]], @"Invalid JSON envelope");
  return [FlutterError errorWithCode:array[0] message:array[1] details:array[2]];
}
@end
