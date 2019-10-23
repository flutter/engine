// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMouseCursorPlugin.h"

#import <objc/message.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kSetAsSystemCursorMethod = @"setAsSystemCursor";
static NSString* const kSystemConstantKey = @"systemConstant";
static NSString* const kHiddenMethod = @"hidden";
static NSString* const kShowMethod = @"show";

static int const kArrowSystemCursor = 0x0001;
static int const kIBeamSystemCursor = 0x0002;
static int const kCrosshairSystemCursor = 0x0003;
static int const kClosedHandSystemCursor = 0x0004;
static int const kOpenHandSystemCursor = 0x0005;
static int const kPointingHandSystemCursor = 0x0006;
static int const kResizeLeftSystemCursor = 0x0007;
static int const kResizeRightSystemCursor = 0x0008;
static int const kResizeLeftRightSystemCursor = 0x0009;
static int const kResizeUpSystemCursor = 0x000a;
static int const kResizeDownSystemCursor = 0x000b;
static int const kResizeUpDownSystemCursor = 0x000c;
static int const kDisappearingItemSystemCursor = 0x000d;
static int const kIBeamCursorForVerticalLayoutSystemCursor = 0x000e;
static int const kDragLinkSystemCursor = 0x000f;
static int const kDragCopySystemCursor = 0x0010;
static int const kContextualMenuSystemCursor = 0x0011;

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
    _mouseCursorContext = [[NSMouseCursorContext alloc] initWithClient:self];
  }
  return self;
}

#pragma mark - Private

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  BOOL handled = YES;
  NSString* method = call.method;
  if ([method isEqualToString:kSetAsSystemCursorMethod]) {
    if (!call.arguments[0]) {
      result([FlutterError
          errorWithCode:@"error"
                message:@"Missing arguments"
                details:@"Missing arguments while trying to set as system mouse cursor"]);
      return;
    }
    NSNumber* systemConstant = call.arguments[0];
    NSCursor cursor = [self resolveSystemCursor]
    if (cursor != nil) {
      [cursor set]
    } else {
      result([FlutterError
          errorWithCode:@"error"
                message:@"Unrecognized arguments"
                details:@"Received unrecognizable systemConstant while trying to set as system mouse cursor"]);
    }
  } else if ([method isEqualToString:kShowMethod]) {
    [NSCursor unhide]
  } else if ([method isEqualToString:kHideMethod]) {
    [NSCursor hide]
  } else {
    handled = NO;
    NSLog(@"Unhandled mouse cursor method '%@'", method);
  }
  result(handled ? nil : FlutterMethodNotImplemented);
}

#pragma mark -
#pragma mark NSCursor

- (nullable NSCursor)resolveSystemCursor:(int)systemConstant {
  switch (systemConstant) {
    case kArrowSystemCursor:
      return [NSCursor arrow];
    // case kIBeamSystemCursor:
    // case kCrosshairSystemCursor:
    // case kClosedHandSystemCursor:
    // case kOpenHandSystemCursor:
    case kPointingHandSystemCursor:
      return [NSCursor pointingHand];
    // case kResizeLeftSystemCursor:
    // case kResizeRightSystemCursor:
    // case kResizeLeftRightSystemCursor:
    // case kResizeUpSystemCursor:
    // case kResizeDownSystemCursor:
    // case kResizeUpDownSystemCursor:
    // case kDisappearingItemSystemCursor:
    // case kIBeamCursorForVerticalLayoutSystemCursor:
    // case kDragLinkSystemCursor:
    // case kDragCopySystemCursor:
    // case kContextualMenuSystemCursor:
    default:
      break;
  }
  return null;
}

@end
