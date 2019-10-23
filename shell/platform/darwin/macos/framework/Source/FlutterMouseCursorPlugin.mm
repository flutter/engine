// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseCursorPlugin.h"

#import <objc/message.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kSystemConstantSetAsMethod = @"setAsSystemCursor";
static NSString* const kSystemConstantKey = @"systemConstant";

static NSString* const kSetHiddenMethod = @"setHidden";
static NSString* const kHiddenKey = @"hidden";

static int const kSystemConstantArrow = 0x0001;
static int const kSystemConstantIBeam = 0x0002;
static int const kSystemConstantCrosshair = 0x0003;
static int const kSystemConstantClosedHand = 0x0004;
static int const kSystemConstantOpenHand = 0x0005;
static int const kSystemConstantPointingHand = 0x0006;
static int const kSystemConstantResizeLeft = 0x0007;
static int const kSystemConstantResizeRight = 0x0008;
static int const kSystemConstantResizeLeftRight = 0x0009;
static int const kSystemConstantResizeUp = 0x000a;
static int const kSystemConstantResizeDown = 0x000b;
static int const kSystemConstantResizeUpDown = 0x000c;
static int const kSystemConstantDisappearingItem = 0x000d;
static int const kSystemConstantIBeamCursorForVerticalLayout = 0x000e;
static int const kSystemConstantOperationNotAllowed = 0x000f;
static int const kSystemConstantDragLink = 0x0010;
static int const kSystemConstantDragCopy = 0x0011;
static int const kSystemConstantContextualMenu = 0x0012;

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
  if ([method isEqualToString:kSystemConstantSetAsMethod]) {
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
  NSNumber* systemConstant = arguments[kSystemConstantKey];
  if (!systemConstant) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument systemConstant while trying to set as system mouse cursor"];
  }
  NSCursor* cursor = [self resolveSystemCursor:[systemConstant intValue]];
  if (cursor == nil) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Unrecognized argument"
              details:@"Received unrecognizable systemConstant while trying to set as system mouse cursor"];
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

- (nullable NSCursor*)resolveSystemCursor:(int)systemConstant {
  switch (systemConstant) {
    case kSystemConstantArrow:
      return [NSCursor arrowCursor];
    case kSystemConstantIBeam:
      return [NSCursor IBeamCursor];
    case kSystemConstantCrosshair:
      return [NSCursor crosshairCursor];
    case kSystemConstantClosedHand:
      return [NSCursor closedHandCursor];
    case kSystemConstantOpenHand:
      return [NSCursor openHandCursor];
    case kSystemConstantPointingHand:
      return [NSCursor pointingHandCursor];
    case kSystemConstantResizeLeft:
      return [NSCursor resizeLeftCursor];
    case kSystemConstantResizeRight:
      return [NSCursor resizeRightCursor];
    case kSystemConstantResizeLeftRight:
      return [NSCursor resizeLeftRightCursor];
    case kSystemConstantResizeUp:
      return [NSCursor resizeUpCursor];
    case kSystemConstantResizeDown:
      return [NSCursor resizeDownCursor];
    case kSystemConstantResizeUpDown:
      return [NSCursor resizeUpDownCursor];
    case kSystemConstantDisappearingItem:
      return [NSCursor disappearingItemCursor];
    case kSystemConstantIBeamCursorForVerticalLayout:
      return [NSCursor IBeamCursorForVerticalLayout];
    case kSystemConstantOperationNotAllowed:
      return [NSCursor operationNotAllowedCursor];
    case kSystemConstantDragLink:
      return [NSCursor dragLinkCursor];
    case kSystemConstantDragCopy:
      return [NSCursor dragCopyCursor];
    case kSystemConstantContextualMenu:
      return [NSCursor contextualMenuCursor];
    default:
      break;
  }
  return nil;
}

@end
