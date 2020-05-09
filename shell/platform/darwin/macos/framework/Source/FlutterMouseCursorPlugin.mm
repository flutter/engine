// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "FlutterMouseCursorPlugin.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kActivateSystemCursorMethod = @"activateSystemCursor";
static NSString* const kKindKey = @"kind";
static NSString* const kDeviceKey = @"device";

static NSString* const kKindValueNone = @"none";

static NSMutableDictionary* cachedSystemCursors;

// Maps a Flutter's constant to a platform's cursor object.
//
// Returns the arrow cursor for unknown constants, including kSystemShapeNone.
static NSCursor* mapKindToCursor(NSString* kind) {
  // The following mapping must be kept in sync with Flutter framework's
  // mouse_cursor.dart
  if ([kind isEqualToString:@"basic"])
    return [NSCursor arrowCursor];
  else if ([kind isEqualToString:@"click"])
    return [NSCursor pointingHandCursor];
  else if ([kind isEqualToString:@"text"])
    return [NSCursor IBeamCursor];
  else if ([kind isEqualToString:@"forbidden"])
    return [NSCursor operationNotAllowedCursor];
  else if ([kind isEqualToString:@"grab"])
    return [NSCursor openHandCursor];
  else if ([kind isEqualToString:@"grabbing"])
    return [NSCursor closedHandCursor];
  else
    return [NSCursor arrowCursor];
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
  NSString* kindArg = arguments[kKindKey];
  NSNumber* deviceArg = arguments[kDeviceKey];
  if (!kindArg || !deviceArg) {
    return [FlutterError
        errorWithCode:@"error"
              message:@"Missing argument"
              details:@"Missing argument while trying to activate system cursor"];
  }
  if ([kindArg isEqualToString:kKindValueNone]) {
    [self hide];
    return nil;
  }
  NSCursor* shapeObject = [FlutterMouseCursorPlugin resolveKindToCursor:kindArg];
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

+ (NSCursor*)resolveKindToCursor:(NSString*)kind {
  if (cachedSystemCursors == nil) {
    cachedSystemCursors = [NSMutableDictionary dictionary];
  }
  
  NSCursor* cachedValue = [cachedSystemCursors objectForKey:kind];
  if (cachedValue == nil) {
    cachedValue = mapKindToCursor(kind);
    [cachedSystemCursors setValue:cachedValue forKey:kind];
  }
  return cachedValue;
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
