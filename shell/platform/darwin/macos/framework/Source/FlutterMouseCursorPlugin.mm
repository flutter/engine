// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseCursorPlugin.h"

#import <objc/message.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

// System shape are constant integers used by the framework to represent a shape
// of system cursors.
//
// They are intentionally designed to be meaningless integers.
//
// They must be kept in sync with Flutter framework's mouse_cursor.dart
static int const kSystemShapeNone = 0x334c4a4c;
static int const kSystemShapeBasic = 0xf17aaabc;
static int const kSystemShapeClick = 0xa8affc08;
static int const kSystemShapeText = 0x1cb251ec;
static int const kSystemShapeForbidden = 0x7fa3b767;
static int const kSystemShapeGrab = 0x28b91f80;
static int const kSystemShapeGrabbing = 0x6631ce3e;

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kActivateShapeMethod = @"activateShape";
static NSString* const kShapeKey = @"shape";
static NSString* const kDeviceKey = @"device";

@interface FlutterMouseCursorDeviceState : NSObject

- (nonnull instancetype)initWithDevice:(int)device
                                 shape:(nonnull NSCursor*)shape;

- (void)dispose;

- (void)activateShape: (nonnull NSCursor *)targetShape;

@property(readonly) int device;

@property() NSCursor *shape;

@end

@implementation FlutterMouseCursorDeviceState {
  BOOL _hidden;
}

- (nonnull instancetype)initWithDevice:(int)device
                                 shape:(nonnull NSCursor*)shape {
  _device = device;
  _shape = shape;
  _hidden = NO;
  [_shape set];
  return self;
}

- (void)dispose {
  if (_hidden)
    [NSCursor unhide];
}

- (void)activateShape: (nonnull NSCursor *)targetShape {
  if (targetShape == _shape && !_hidden)
    return;
  [targetShape set];
  if (_hidden) {
    [NSCursor unhide];
  }
  _shape = targetShape;
  _hidden = NO;
}

- (void)hide {
  if (!_hidden) {
    [NSCursor hide];
  }
  _hidden = YES;
}

#pragma mark - Private

@end

@interface FlutterMouseCursorPlugin ()
/**
 * The channel used to communicate with Flutter.
 */
@property() FlutterMethodChannel* channel;

/**
 * The FlutterViewController to manage input for.
 */
@property(nonatomic, weak) FlutterViewController* flutterViewController;

@property() NSMutableDictionary *shapeObjects;

@property() FlutterMouseCursorDeviceState *deviceState;

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
  BOOL handled = YES;
  NSString* method = call.method;
  if ([method isEqualToString:kActivateShapeMethod]) {
    result([self activateShape:call.arguments]);
  } else {
    handled = NO;
    NSLog(@"Unhandled mouse cursor method '%@'", method);
    result(FlutterMethodNotImplemented);
  }
}

- (id)activateShape:(nonnull NSDictionary*)arguments {
  if (!arguments) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing arguments"
              details:@"Missing arguments while trying to activate shape"];
  }
  NSNumber* shapeArg = arguments[kShapeKey];
  if (!shapeArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument shape while trying to activate shape"];
  }
  NSNumber* deviceArg = arguments[kDeviceKey];
  if (!deviceArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument device while trying to activate shape"];
  }
  NSNumber *noNoneShape = [shapeArg intValue] == kSystemShapeNone ?
    [NSNumber numberWithInt:kSystemShapeBasic] :
    shapeArg;
  NSCursor* shapeObject = [self resolveShape:noNoneShape];
  if (shapeObject == nil) {
    // Unregistered shape. Return false to request fallback.
    return @(NO);
  }
  [self ensureDevice:deviceArg withShapeObject:shapeObject];
  if ([shapeArg intValue] == kSystemShapeNone) {
    [_deviceState hide];
  } else {
    [_deviceState activateShape:shapeObject];
  }
  return @(YES);
}

- (nullable NSCursor*)resolveShape:(NSNumber*)shape {
  NSCursor *cachedObject = _shapeObjects[shape];
  if (cachedObject)
    return cachedObject;
  NSCursor *systemObject = [FlutterMouseCursorPlugin resolveSystemShape:shape];
  if (!systemObject)
    return nil;
  _shapeObjects[shape] = systemObject;
  return systemObject;
}

- (void)ensureDevice:(NSNumber*)device
     withShapeObject:(nonnull NSCursor*)shapeObject {
  // MacOS only supports one device. Destroy the old when the new one is different.
  if (_deviceState.device != [device intValue]) {
    [_deviceState dispose];
    _deviceState = nil;
  }
  if (!_deviceState) {
    _deviceState = [[FlutterMouseCursorDeviceState alloc] initWithDevice:[device intValue]
                                                                   shape:shapeObject];
  }
}

#pragma mark - Static

// Does not handle kSystemShapeNone.
// Returns null for default.
+ (nullable NSCursor*)resolveSystemShape:(NSNumber*)platformConstant {
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
    default:
      break;
  }
  return nil;
}

@end
