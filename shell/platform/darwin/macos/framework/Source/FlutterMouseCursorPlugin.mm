// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseCursorPlugin.h"

#import <objc/message.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kPlatformConstantSetAsMethod = @"setAsSystemCursor";
static NSString* const kPlatformConstantKey = @"platformConstant";

static NSString* const kSetHiddenMethod = @"setHidden";
static NSString* const kHiddenKey = @"hidden";

static int const kPlatformConstantArrow = 0x0001;
static int const kPlatformConstantIBeam = 0x0002;
static int const kPlatformConstantCrosshair = 0x0003;
static int const kPlatformConstantClosedHand = 0x0004;
static int const kPlatformConstantOpenHand = 0x0005;
static int const kPlatformConstantPointingHand = 0x0006;
static int const kPlatformConstantResizeLeft = 0x0007;
static int const kPlatformConstantResizeRight = 0x0008;
static int const kPlatformConstantResizeLeftRight = 0x0009;
static int const kPlatformConstantResizeUp = 0x000a;
static int const kPlatformConstantResizeDown = 0x000b;
static int const kPlatformConstantResizeUpDown = 0x000c;
static int const kPlatformConstantDisappearingItem = 0x000d;
static int const kPlatformConstantIBeamCursorForVerticalLayout = 0x000e;
static int const kPlatformConstantOperationNotAllowed = 0x000f;
static int const kPlatformConstantDragLink = 0x0010;
static int const kPlatformConstantDragCopy = 0x0011;
static int const kPlatformConstantContextualMenu = 0x0012;

/**
 * Private properties of FlutterMouseCursorPlugin.
 */
@interface FlutterMouseCursorPlugin ()

/**
 * The channel used to communicate with Flutter.
 */
@property(nonatomic) FlutterMethodChannel* channel;

/**
 * The FlutterViewController to manage input for.
 */
@property(nonatomic, weak) FlutterViewController* flutterViewController;

/**
 * Handles a Flutter system message on the mouse cursor channel.
 */
- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;

@end

@implementation FlutterMouseCursorPlugin

- (instancetype)initWithViewController:(FlutterViewController*)viewController {
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
  }
  return self;
}

#pragma mark - Private

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  BOOL handled = YES;
  NSString* method = call.method;
  if ([method isEqualToString:kPlatformConstantSetAsMethod]) {
    result([self setAsSystemCursor:call.arguments]);
  } else if ([method isEqualToString:kSetHiddenMethod]) {
    result([self setHidden:call.arguments]);
  } else {
    handled = NO;
    NSLog(@"Unhandled mouse cursor method '%@'", method);
    result(FlutterMethodNotImplemented);
  }
}

- (id)setAsSystemCursor:(NSDictionary*)arguments {
  if (!arguments) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing arguments"
              details:@"Missing arguments while trying to set as system mouse cursor"];
  }
  NSNumber* platformConstant = arguments[kPlatformConstantKey];
  if (!platformConstant) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument platformConstant while trying to set as system mouse cursor"];
  }
  NSCursor* cursor = [self resolveSystemCursor:[platformConstant intValue]];
  if (cursor == nil) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Unrecognized argument"
              details:@"Received unrecognizable platformConstant while trying to set as system mouse cursor"];
  }
  [cursor set];
  return nil;
}

- (id)setHidden:(NSDictionary*)arguments {
  if (!arguments) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing arguments"
              details:@"Missing arguments while trying to toggle mouse cursor hidden"];
  }
  NSNumber* hidden = arguments[kHiddenKey];
  if (!hidden) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument hidden while trying to set as system mouse cursor"];
  }
  if ([hidden boolValue]) {
    [NSCursor hide];
  } else {
    [NSCursor unhide];
  }
  return nil;
}

#pragma mark -
#pragma mark NSCursor

- (nullable NSCursor*)resolveSystemCursor:(int)platformConstant {
  switch (platformConstant) {
    case kPlatformConstantArrow:
      return [NSCursor arrowCursor];
    case kPlatformConstantIBeam:
      return [NSCursor IBeamCursor];
    case kPlatformConstantCrosshair:
      return [NSCursor crosshairCursor];
    case kPlatformConstantClosedHand:
      return [NSCursor closedHandCursor];
    case kPlatformConstantOpenHand:
      return [NSCursor openHandCursor];
    case kPlatformConstantPointingHand:
      return [NSCursor pointingHandCursor];
    case kPlatformConstantResizeLeft:
      return [NSCursor resizeLeftCursor];
    case kPlatformConstantResizeRight:
      return [NSCursor resizeRightCursor];
    case kPlatformConstantResizeLeftRight:
      return [NSCursor resizeLeftRightCursor];
    case kPlatformConstantResizeUp:
      return [NSCursor resizeUpCursor];
    case kPlatformConstantResizeDown:
      return [NSCursor resizeDownCursor];
    case kPlatformConstantResizeUpDown:
      return [NSCursor resizeUpDownCursor];
    case kPlatformConstantDisappearingItem:
      return [NSCursor disappearingItemCursor];
    case kPlatformConstantIBeamCursorForVerticalLayout:
      return [NSCursor IBeamCursorForVerticalLayout];
    case kPlatformConstantOperationNotAllowed:
      return [NSCursor operationNotAllowedCursor];
    case kPlatformConstantDragLink:
      return [NSCursor dragLinkCursor];
    case kPlatformConstantDragCopy:
      return [NSCursor dragCopyCursor];
    case kPlatformConstantContextualMenu:
      return [NSCursor contextualMenuCursor];
    default:
      break;
  }
  return nil;
}

@end
