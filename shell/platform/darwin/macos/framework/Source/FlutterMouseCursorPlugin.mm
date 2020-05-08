// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseCursorPlugin.h"

#import <objc/message.h>

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
// Returns nil for unknown constants, including kSystemShapeNone.
static NSCursor* resolveSystemShape(NSNumber* platformConstant) {
  switch ([platformConstant intValue]) {
    case kSystemShapeBasic:
      return [NSCursor arrowCursor];
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
  return nil;
}

@interface FlutterMouseCursorPlugin ()
/**
 * The channel used to communicate with Flutter.
 */
@property(nonatomic) FlutterMethodChannel* channel;

/**
 * The FlutterViewController to manage input for.
 */
@property(nonatomic, weak) FlutterViewController* flutterViewController;

@property(nonatomic) NSMutableDictionary *shapeObjects;

@property(nonatomic) NSCursor *shape;

@property(nonatomic) BOOL hidden;

@end

@implementation FlutterMouseCursorPlugin

- (instancetype)initWithViewController:(nonnull FlutterViewController*)viewController {
  self = [super init];
  if (self != nil) {
    _flutterViewController = viewController;
    _channel = [FlutterMethodChannel methodChannelWithName:kMouseCursorChannel
                                           binaryMessenger:viewController.engine.binaryMessenger
                                                     codec:[FlutterStandardMethodCodec sharedInstance]];
    __weak FlutterMouseCursorPlugin* weakSelf = self;
    [_channel setMethodCallHandler:^(FlutterMethodCall* call, FlutterResult result) {
      [weakSelf handleMethodCall:call result:result];
    }];
    _shapeObjects = [[NSMutableDictionary alloc] init];
  }
  return self;
}

#pragma mark - Private

- (void)handleMethodCall:(nonnull FlutterMethodCall*)call result:(FlutterResult)result {
  NSString* method = call.method;
  if ([method isEqualToString:kActivateSystemCursorMethod]) {
    result([self activateSystemCursor:call.arguments]);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (id)activateSystemCursor:(nonnull NSDictionary*)arguments {
  if (!arguments) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing arguments"
              details:@"Missing arguments while trying to activate system cursor"];
  }
  NSNumber* shapeCodeArg = arguments[kShapeCodeKey];
  if (!shapeCodeArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument shapeCode while trying to activate system cursor"];
  }
  NSNumber* deviceArg = arguments[kDeviceKey];
  if (!deviceArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument device while trying to activate system cursor"];
  }
  if ([shapeCodeArg intValue] == kSystemShapeNone) {
    return [self hide];
  }
  NSCursor* shapeObject = [self resolveShapeCode:shapeCodeArg];
  if (shapeObject == nil) {
    // Unregistered shape. Return false to request fallback.
    return @(NO);
  }
  return [self activateShapeObject:shapeObject];
}

- (nullable NSCursor*)resolveShapeCode:(NSNumber*)shape {
  NSCursor *cachedObject = _shapeObjects[shape];
  if (cachedObject)
    return cachedObject;
  NSCursor *systemObject = resolveSystemShape(shape);
  if (!systemObject)
    return nil;
  _shapeObjects[shape] = systemObject;
  return systemObject;
}

- (id)activateShapeObject: (nonnull NSCursor *)targetShape {
  if (targetShape == _shape && !_hidden)
    return @(YES);
  [targetShape set];
  if (_hidden) {
    [NSCursor unhide];
  }
  _shape = targetShape;
  _hidden = NO;
  return @(YES);
}

- (id)hide {
  if (!_hidden) {
    [NSCursor hide];
  }
  _hidden = YES;
  return @(YES);
}

@end
