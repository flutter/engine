// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "FlutterMouseCursorPlugin.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"

static NSString* const kMouseCursorChannel = @"flutter/mousecursor";

static NSString* const kActivateSystemCursorMethod = @"activateSystemCursor";
static NSString* const kKindKey = @"kind";

static NSString* const kKindValueNone = @"none";

static NSDictionary* systemCursors;

// The following code uses private interface to CoreCursor. This should be safe,
// as it is used by Chromium as well.
// https://github.com/chromium/chromium/blob/466db17cd64768dec52e9dae71531c53590d583c/ui/base/cocoa/cursor_utils.mm

enum {
  // kArrowCursor = 0,                // [NSCursor arrowCursor]
  // kIBeamCursor = 1,                // [NSCursor IBeamCursor]
  // kMakeAliasCursor = 2,            // [NSCursor dragLinkCursor]
  // kOperationNotAllowedCursor = 3,  // [NSCursor operationNotAllowedCursor]
  kBusyButClickableCursor = 4,
  // kCopyCursor = 5,                 // [NSCursor dragCopyCursor]
  // kClosedHandCursor = 11,          // [NSCursor closedHandCursor]
  // kOpenHandCursor = 12,            // [NSCursor openHandCursor]
  // kPointingHandCursor = 13,        // [NSCursor pointingHandCursor]

  // The following three cursor types are technically available, but we do not
  // plan to implement them unless explicitly demanded. This is because that
  // during testing they have an issue of not animating unless the mouse is
  // moving, and also they're exclusively present on macOS and so rare.
  // See: https://github.com/flutter/flutter/issues/139692
  // kCountingUpHandCursor = 14,
  // kCountingDownHandCursor = 15,
  // kCountingUpAndDownHandCursor = 16,

  // kResizeLeftCursor = 17,          // [NSCursor resizeLeftCursor]
  // kResizeRightCursor = 18,         // [NSCursor resizeRightCursor]
  // kResizeLeftRightCursor = 19,     // [NSCursor resizeLeftRightCursor]
  // kCrosshairCursor = 20,           // [NSCursor crosshairCursor]
  // kResizeUpCursor = 21,            // [NSCursor resizeUpCursor]
  // kResizeDownCursor = 22,          // [NSCursor resizeDownCursor]
  // kResizeUpDownCursor = 23,        // [NSCursor resizeUpDownCursor]
  // kContextualMenuCursor = 24,      // [NSCursor contextualMenuCursor]
  // kDisappearingItemCursor = 25,    // [NSCursor disappearingItemCursor]
  // kVerticalIBeamCursor = 26,       // [NSCursor IBeamCursorForVerticalLayout]
  kResizeEastCursor = 27,
  kResizeEastWestCursor = 28,
  kResizeNortheastCursor = 29,
  kResizeNortheastSouthwestCursor = 30,
  kResizeNorthCursor = 31,
  kResizeNorthSouthCursor = 32,
  kResizeNorthwestCursor = 33,
  kResizeNorthwestSoutheastCursor = 34,
  kResizeSoutheastCursor = 35,
  kResizeSouthCursor = 36,
  kResizeSouthwestCursor = 37,
  kResizeWestCursor = 38,
  kMoveCursor = 39,
  kHelpCursor = 40,
  kCellCursor = 41,
  kZoomInCursor = 42,
  kZoomOutCursor = 43
};

using CrCoreCursorType = int64_t;

@interface CrCoreCursor : NSCursor {
 @private
  CrCoreCursorType type_;
}

+ (id)cursorWithType:(CrCoreCursorType)type;
- (id)initWithType:(CrCoreCursorType)type;
- (CrCoreCursorType)_coreCursorType;
@end

@implementation CrCoreCursor
+ (id)cursorWithType:(CrCoreCursorType)type {
  NSCursor* cursor = [[CrCoreCursor alloc] initWithType:type];
  if ([cursor image]) {
    return cursor;
  }
  return nil;
}

- (id)initWithType:(CrCoreCursorType)type {
  if ((self = [super init])) {
    type_ = type;
  }
  return self;
}

- (CrCoreCursorType)_coreCursorType {
  return type_;
}
@end

/**
 * Maps a Flutter's constant to a platform's cursor object.
 *
 * Returns the arrow cursor for unknown constants, including kKindValueNone.
 */
static NSCursor* GetCursorForKind(NSString* kind) {
  // The following list of keys must be kept in sync with Flutter framework's
  // mouse_cursor.dart

  if (systemCursors == nil) {
    systemCursors = @{
      @"alias" : [NSCursor dragLinkCursor],
      @"allScroll" : [CrCoreCursor cursorWithType:kMoveCursor],
      @"basic" : [NSCursor arrowCursor],
      @"cell" : [CrCoreCursor cursorWithType:kCellCursor],
      @"click" : [NSCursor pointingHandCursor],
      @"contextMenu" : [NSCursor contextualMenuCursor],
      @"copy" : [NSCursor dragCopyCursor],
      @"disappearing" : [NSCursor disappearingItemCursor],
      @"forbidden" : [NSCursor operationNotAllowedCursor],
      @"grab" : [NSCursor openHandCursor],
      @"grabbing" : [NSCursor closedHandCursor],
      @"help" : [CrCoreCursor cursorWithType:kHelpCursor],
      @"move" : [CrCoreCursor cursorWithType:kMoveCursor],
      // @"none": (see kKindValueNone)
      @"noDrop" : [NSCursor operationNotAllowedCursor],
      @"precise" : [NSCursor crosshairCursor],
      @"progress" : [CrCoreCursor cursorWithType:kBusyButClickableCursor],

      @"resizeColumn" : [NSCursor resizeLeftRightCursor],
      @"resizeDown" : [NSCursor resizeDownCursor],
      @"resizeLeft" : [NSCursor resizeLeftCursor],
      @"resizeLeftRight" : [NSCursor resizeLeftRightCursor],
      @"resizeRight" : [NSCursor resizeRightCursor],
      @"resizeRow" : [NSCursor resizeUpDownCursor],
      @"resizeUp" : [NSCursor resizeUpCursor],
      @"resizeUpDown" : [NSCursor resizeUpDownCursor],

      @"text" : [NSCursor IBeamCursor],
      @"verticalText" : [NSCursor IBeamCursorForVerticalLayout],
      @"wait" : [CrCoreCursor cursorWithType:kBusyButClickableCursor],
      @"zoomIn" : [CrCoreCursor cursorWithType:kZoomInCursor],
      @"zoomOut" : [CrCoreCursor cursorWithType:kZoomOutCursor],
    };
  }
  NSCursor* result = [systemCursors objectForKey:kind];
  if (result == nil) {
    return [NSCursor arrowCursor];
  }
  return result;
}

@interface FlutterMouseCursorPlugin ()
/**
 * Whether the cursor is currently hidden.
 */
@property(nonatomic) BOOL hidden;

/**
 * Handles the method call that activates a system cursor.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns nil.
 */
- (FlutterError*)activateSystemCursor:(nonnull NSDictionary*)arguments;

/**
 * Displays the specified cursor.
 *
 * Unhides the cursor before displaying the cursor, and updates
 * internal states.
 */
- (void)displayCursorObject:(nonnull NSCursor*)cursorObject;

/**
 * Hides the cursor.
 */
- (void)hide;

/**
 * Handles all method calls from Flutter.
 */
- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;

@end

@implementation FlutterMouseCursorPlugin

#pragma mark - Private

NSMutableDictionary* cachedSystemCursors;

- (instancetype)init {
  self = [super init];
  if (self) {
    cachedSystemCursors = [NSMutableDictionary dictionary];
  }
  return self;
}

- (void)dealloc {
  if (_hidden) {
    [NSCursor unhide];
  }
}

- (FlutterError*)activateSystemCursor:(nonnull NSDictionary*)arguments {
  NSString* kindArg = arguments[kKindKey];
  if (!kindArg) {
    return [FlutterError errorWithCode:@"error"
                               message:@"Missing argument"
                               details:@"Missing argument while trying to activate system cursor"];
  }
  if ([kindArg isEqualToString:kKindValueNone]) {
    [self hide];
    return nil;
  }
  NSCursor* cursorObject = [FlutterMouseCursorPlugin cursorFromKind:kindArg];
  [self displayCursorObject:cursorObject];
  return nil;
}

- (void)displayCursorObject:(nonnull NSCursor*)cursorObject {
  [cursorObject set];
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

+ (NSCursor*)cursorFromKind:(NSString*)kind {
  NSCursor* cachedValue = [cachedSystemCursors objectForKey:kind];
  if (!cachedValue) {
    cachedValue = GetCursorForKind(kind);
    [cachedSystemCursors setValue:cachedValue forKey:kind];
  }
  return cachedValue;
}

#pragma mark - FlutterPlugin implementation

+ (void)registerWithRegistrar:(id<FlutterPluginRegistrar>)registrar {
  FlutterMethodChannel* channel = [FlutterMethodChannel methodChannelWithName:kMouseCursorChannel
                                                              binaryMessenger:registrar.messenger];
  FlutterMouseCursorPlugin* instance = [[FlutterMouseCursorPlugin alloc] init];
  [registrar addMethodCallDelegate:instance channel:channel];
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  NSString* method = call.method;
  if ([method isEqualToString:kActivateSystemCursorMethod]) {
    result([self activateSystemCursor:call.arguments]);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

@end
