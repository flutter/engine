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

static NSString* const kCreateImageCursorMethod = @"createImageCursor";
static NSString* const kDataKey = @"data";
static NSString* const kWidthKey = @"width";
static NSString* const kHeightKey = @"height";
static NSString* const kOffsetXKey = @"offsetX";
static NSString* const kOffsetYKey = @"offsetY";

static NSString* const kActivateImageCursorMethod = @"activateImageCursor";
static NSString* const kCursorIdKey = @"cursorId";

static NSDictionary* systemCursors;

/**
 * Maps a Flutter's constant to a platform's cursor object.
 *
 * Returns the arrow cursor for unknown constants, including kSystemShapeNone.
 */
static NSCursor* GetCursorForKind(NSString* kind) {
  // The following mapping must be kept in sync with Flutter framework's
  // mouse_cursor.dart

  if (systemCursors == nil) {
    systemCursors = @{
      @"alias" : [NSCursor dragLinkCursor],
      @"basic" : [NSCursor arrowCursor],
      @"click" : [NSCursor pointingHandCursor],
      @"contextMenu" : [NSCursor contextualMenuCursor],
      @"copy" : [NSCursor dragCopyCursor],
      @"disappearing" : [NSCursor disappearingItemCursor],
      @"forbidden" : [NSCursor operationNotAllowedCursor],
      @"grab" : [NSCursor openHandCursor],
      @"grabbing" : [NSCursor closedHandCursor],
      @"noDrop" : [NSCursor operationNotAllowedCursor],
      @"precise" : [NSCursor crosshairCursor],
      @"text" : [NSCursor IBeamCursor],
      @"resizeColumn" : [NSCursor resizeLeftRightCursor],
      @"resizeDown" : [NSCursor resizeDownCursor],
      @"resizeLeft" : [NSCursor resizeLeftCursor],
      @"resizeLeftRight" : [NSCursor resizeLeftRightCursor],
      @"resizeRight" : [NSCursor resizeRightCursor],
      @"resizeRow" : [NSCursor resizeUpDownCursor],
      @"resizeUp" : [NSCursor resizeUpCursor],
      @"resizeUpDown" : [NSCursor resizeUpDownCursor],
      @"verticalText" : [NSCursor IBeamCursorForVerticalLayout],
    };
  }
  NSCursor* result = [systemCursors objectForKey:kind];
  if (result == nil)
    return [NSCursor arrowCursor];
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
 * Creates an image cursor and returns an identifier.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns the cursor ID.
 */
- (id)createImageCursor:(nonnull NSDictionary*)arguments;

/**
 * Handles the method call that activates an image cursor.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns nil.
 */
- (FlutterError*)activateImageCursor:(nonnull NSDictionary*)arguments;

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

NSMutableDictionary* imageCursors;

uint32_t latestImageId;

- (instancetype)init {
  self = [super init];
  if (self) {
    cachedSystemCursors = [NSMutableDictionary dictionary];
    latestImageId = 1;
    imageCursors = [NSMutableDictionary dictionary];
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

- (id)createImageCursor:(nonnull NSDictionary*)arguments {
  FlutterStandardTypedData* dataArg = arguments[kDataKey];
  NSNumber* heightArg = arguments[kHeightKey];
  NSNumber* widthArg = arguments[kWidthKey];
  NSNumber* offsetXArg = arguments[kOffsetXKey];
  NSNumber* offsetYArg = arguments[kOffsetYKey];
  if (!dataArg || !heightArg || !widthArg || !offsetXArg || !offsetYArg) {
    return [FlutterError errorWithCode:@"error"
                               message:@"Missing argument"
                               details:@"Missing argument while trying to activate system cursor"];
  }
  NSData* data = [dataArg data];
  NSUInteger length = [data length];
  NSUInteger width = [widthArg unsignedLongValue];
  NSUInteger height = [heightArg unsignedLongValue];
  const uint8_t bytesPerPixel = 4;  // RGBA
  NSLog(@"DataLength %lu height %@ width %@ offsetX %@ offsetY %@", length, heightArg, widthArg,
        offsetXArg, offsetYArg);
  NSAssert(length == height * width * bytesPerPixel, @"Image size mismatch");

  NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc]
      initWithBitmapDataPlanes:nil  // Allocate empty space to assign later
                    pixelsWide:[widthArg unsignedLongValue]
                    pixelsHigh:[heightArg unsignedLongValue]
                 bitsPerSample:8
               samplesPerPixel:bytesPerPixel
                      hasAlpha:YES
                      isPlanar:NO
                colorSpaceName:NSDeviceRGBColorSpace
                  bitmapFormat:0  // RGBA, premultiplied
                   bytesPerRow:0  // Auto decide
                  bitsPerPixel:0  // Auto decide
  ];
  // Every pixel is 4 bytes.
  const uint32_t* sourcePixel = reinterpret_cast<const uint32_t*>(data.bytes);
  unsigned char* targetRowStart = imageRep.bitmapData;
  for (size_t row_id = 0; row_id < height; row_id += 1) {
    uint32_t* targetPixel = reinterpret_cast<uint32_t*>(targetRowStart);
    for (size_t pixel_id = 0; pixel_id < width; pixel_id += 1) {
      *targetPixel = *sourcePixel;
      ++targetPixel;
      ++sourcePixel;
    }
    targetRowStart += imageRep.bytesPerRow;
  }
  NSImage* image = [[NSImage alloc] initWithCGImage:[imageRep CGImage]
                                               size:NSMakeSize(width, height)];
  NSCursor* cursorObject = [[NSCursor alloc]
      initWithImage:image
            hotSpot:NSMakePoint([offsetXArg doubleValue], [offsetYArg doubleValue])];

  latestImageId += 1;
  imageCursors[@(latestImageId)] = cursorObject;
  return @(latestImageId);
}

- (FlutterError*)activateImageCursor:(nonnull NSDictionary*)arguments {
  NSNumber* cursorIdArg = arguments[kCursorIdKey];
  if (!cursorIdArg) {
    return [FlutterError errorWithCode:@"error"
                               message:@"Missing argument"
                               details:@"Missing argument while trying to activate system cursor"];
  }
  NSCursor* cursorObject = imageCursors[cursorIdArg];
  if (cursorObject == nil) {
    NSLog(@"Unexisting image mouse cursor ID %@", cursorIdArg);
  } else {
    [self displayCursorObject:cursorObject];
  }
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
  } else if ([method isEqualToString:kCreateImageCursorMethod]) {
    result([self createImageCursor:call.arguments]);
  } else if ([method isEqualToString:kActivateImageCursorMethod]) {
    result([self activateImageCursor:call.arguments]);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

@end
