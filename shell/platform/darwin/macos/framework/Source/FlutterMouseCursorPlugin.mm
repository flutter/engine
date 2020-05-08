// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "FlutterMouseCursorPlugin.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"

// System shape are constant integers used by the framework to represent a shape
// of system cursors.
//
// They are intentionally designed to be meaningless integers.
//
// They must be kept in sync with Flutter framework's mouse_cursor.dart
static const int kSystemShapeNone = 0x334c4a;
static const int kSystemShapeBasic = 0xf17aaa;
static const int kSystemShapeClick = 0xa8affc;
static const int kSystemShapeText = 0x1cb251;
static const int kSystemShapeForbidden = 0x350f9d;
static const int kSystemShapeGrab = 0x28b91f;
static const int kSystemShapeGrabbing = 0x6631ce;

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kActivateSystemCursorMethod = @"activateSystemCursor";
static NSString* const kShapeCodeKey = @"shapeCode";
static NSString* const kDeviceKey = @"device";

// Maps a Flutter's constant to a platform's cursor object.
//
// Returns the arrow cursor for unknown constants, including kSystemShapeNone.
static NSCursor* resolveSystemShape(NSNumber* platformConstant) {
  switch ([platformConstant intValue]) {
    case kSystemShapeBasic:
      break;
    case kSystemShapeClick:
      return [NSCursor pointingHandCursor];
    case kSystemShapeText:
      return [NSCursor IBeamCursor];
    case kSystemShapeForbidden:
      return [NSCursor operationNotAllowedCursor];
    case kSystemShapeGrab:
      return [NSCursor openHandCursor];
    case kSystemShapeGrabbing:
      return [NSCursor closedHandCursor];
  }
  return [NSCursor arrowCursor];;
}

@interface FlutterMouseCursorPlugin ()
/**
 * The channel used to communicate with Flutter.
 */
@property(nonatomic) FlutterMethodChannel* channel;

/**
 * Whether the cursor is currently hidden. 
 */
@property(nonatomic) BOOL hidden;

@end

@implementation FlutterMouseCursorPlugin

- (instancetype)initWithChannel:(FlutterMethodChannel *)channel {
  self = [super init];
  if (self) {
    _channel = channel;
  }
  return self;
}

#pragma mark - Private

- (id)activateSystemCursor:(nonnull NSDictionary*)arguments {
  if (!arguments) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing arguments"
              details:@"Missing arguments while trying to activate system cursor"];
  }
  NSNumber* shapeCodeArg = arguments[kShapeCodeKey];
  NSNumber* deviceArg = arguments[kDeviceKey];
  if (!shapeCodeArg || !deviceArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument while trying to activate system cursor"];
  }
  if ([shapeCodeArg intValue] == kSystemShapeNone) {
    [self hide];
    return nil;
  }
  NSCursor* shapeObject = resolveSystemShape(shapeCodeArg);
  [self activateShapeObject:shapeObject];
  return nil;
}

- (void)activateShapeObject: (nonnull NSCursor *)targetShape {
  [targetShape set];
  if (_hidden) {
    [NSCursor unhide];
  }
  _hidden = NO;
}

- (void)hide {
  if (!_hidden) {
    [NSCursor hide];
  }
  _hidden = YES;
}

#pragma mark - FlutterPlugin implementation

+ (void)registerWithRegistrar:(id<FlutterPluginRegistrar>)registrar {
  FlutterMethodChannel *channel = [FlutterMethodChannel methodChannelWithName:kMouseCursorChannel
                                                              binaryMessenger:registrar.messenger];
  FlutterMouseCursorPlugin *instance = [[FlutterMouseCursorPlugin alloc] initWithChannel:channel];
  [registrar addMethodCallDelegate:instance channel:channel];
}

- (void)handleMethodCall:(FlutterMethodCall *)call result:(FlutterResult)result {
  NSString* method = call.method;
  if ([method isEqualToString:kActivateSystemCursorMethod]) {
    result([self activateSystemCursor:call.arguments]);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

@end
