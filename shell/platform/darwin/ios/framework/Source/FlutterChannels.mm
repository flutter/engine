// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterChannels.h"

#pragma mark - Basic message channel

@implementation FlutterMessageChannel {
  FlutterViewController* _controller;
  NSString* _name;
  NSObject<FlutterMessageCodec>* _codec;
}
+ (instancetype) withController:(FlutterViewController*)controller
                           name:(NSString*)name
                          codec:(NSObject<FlutterMessageCodec>*)codec {
  return [[[FlutterMessageChannel alloc] initWithController:controller name:name codec:codec] autorelease];
}

- (instancetype) initWithController:(FlutterViewController*)controller
                               name:(NSString*)name
                              codec:(NSObject<FlutterMessageCodec>*)codec {
  if (self = [super init]) {
    _controller = [controller retain];
    _name = [name retain];
    _codec = [codec retain];
  }
  return self;
}

- (void) dealloc {
  [_controller release];
  [_name release];
  [_codec release];
  [super dealloc];
}

- (void) send:(id)message andHandleReplyWith:(FlutterReplyHandler)handler {
  [_controller sendBinaryMessage:[_codec encode:message]
                       onChannel: _name
              andHandleReplyWith:
    ^(NSData *reply) {
      if (handler)
        handler([_codec decode:reply]);
    }
  ];
}

- (void) handleMessagesWith:(FlutterMessageHandler)handler {
  if (handler) {
    [_controller handleBinaryMessagesOnChannel: _name
                                   withHandler:
      ^(NSData* message, FlutterBinaryReplyHandler replyHandler) {
        handler([_codec decode:message], ^(id reply) {
          replyHandler([_codec encode:reply]);
        });
      }
    ];
  } else {
    [_controller handleBinaryMessagesOnChannel: _name
                                   withHandler: nil];
  }
}
@end

#pragma mark - Method channel

@implementation FlutterError
+ (instancetype) withCode:(NSString*)code message:(NSString*)message details:(id)details {
  return [[[FlutterError alloc] initWithCode:code message:message details:details] autorelease];
}

- (instancetype) initWithCode:(NSString*)code message:(NSString*)message details:(id)details {
  if (self = [super init]) {
    _code = [code retain];
    _message = [message retain];
    _details = [details retain];
  }
  return self;
}

- (void) dealloc {
  [_code release];
  [_message release];
  [_details release];
  [super dealloc];
}
@end

@implementation FlutterMethodCall
+ (instancetype) withMethod:(NSString*)method andArguments:(id)arguments {
  return [[[FlutterMethodCall alloc] initWithMethod:method andArguments:arguments] autorelease];
}

- (instancetype) initWithMethod:(NSString*)method andArguments:(id)arguments {
  if (self = [super init]) {
    _method = [method retain];
    _arguments = [arguments retain];
  }
  return self;
}

- (void)dealloc {
  [_method release];
  [_arguments release];
  [super dealloc];
}
@end

@implementation FlutterMethodChannel {
  FlutterViewController* _controller;
  NSString* _name;
  NSObject<FlutterMethodCodec>* _codec;
}

+ (instancetype) withController:(FlutterViewController*)controller
                           name:(NSString*)name
                          codec:(NSObject<FlutterMethodCodec>*)codec {
  return [[[FlutterMethodChannel alloc] initWithController:controller name:name codec:codec] autorelease];
}

- (instancetype) initWithController:(FlutterViewController*)controller
                               name:(NSString*)name
                              codec:(NSObject<FlutterMethodCodec>*)codec {
  if (self = [super init]) {
    _controller = [controller retain];
    _name = [name retain];
    _codec = [codec retain];
  }
  return self;
}

- (void) dealloc {
  [_controller release];
  [_name release];
  [_codec release];
  [super dealloc];
}

- (void) handleMethodCallsWith:(FlutterMethodCallHandler)handler {
  [_controller
    handleBinaryMessagesOnChannel:_name
                      withHandler:^(NSData* message, FlutterBinaryReplyHandler reply) {
      FlutterMethodCall* call = [_codec decodeMethodCall:message];
      handler(call, ^(id result, FlutterError* error) {
        if (error)
          reply([_codec encodeErrorEnvelope:error]);
        else
          reply([_codec encodeSuccessEnvelope:result]);
      });
    }
  ];
}

- (void) handleStreamWith:(FlutterStreamHandler)handler {
  [_controller
    handleBinaryMessagesOnChannel:_name
                      withHandler:^(NSData* message, FlutterBinaryReplyHandler reply) {
      FlutterMethodCall* call = [_codec decodeMethodCall:message];
      handler(call, ^(id result, FlutterError* error) {
        if (error)
          reply([_codec encodeErrorEnvelope:error]);
        else
          reply([_codec encodeSuccessEnvelope:nil]);
        },
        ^(id event, FlutterError* error, BOOL done) {
          if (error)
            [_controller sendBinaryMessage:[_codec encodeErrorEnvelope:error] onChannel:_name andHandleReplyWith:nil];
          else if (done)
            [_controller sendBinaryMessage:nil onChannel:_name andHandleReplyWith:nil];
          else
            [_controller sendBinaryMessage:[_codec encodeSuccessEnvelope:event] onChannel:_name andHandleReplyWith:nil];
        }
      );
    }
  ];
}
@end
