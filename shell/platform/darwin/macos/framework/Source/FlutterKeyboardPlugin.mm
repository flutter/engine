// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/embedder/embedder.h"
#import "FlutterKeyboardPlugin.h"
#import "KeyCodeMap_internal.h"

@interface FlutterKeyboardPlugin ()

/**
 * The FlutterViewController to manage input for.
 */
@property(nonatomic, weak) FlutterViewController* flutterViewController;

/**
 * Handles the method call that activates a system cursor.
 *
 * Returns a FlutterError if the arguments can not be recognized. Otherwise
 * returns nil.
 */
- (void)dispatchEvent:(nonnull NSEvent*)event;

@end

@implementation FlutterKeyboardPlugin

- (instancetype)initWithViewController:(FlutterViewController*)viewController {
  self = [super init];
  if (self != nil) {
    _flutterViewController = viewController;
  }
  return self;
}

- (void)dispatchEvent:(NSEvent*)event {
  FlutterLogicalKeyEvent logicalEvent = {
      .struct_size = sizeof(FlutterLogicalKeyEvent),
      .kind = event.type == NSEventTypeKeyDown ? kFlutterKeyEventKindDown : kFlutterKeyEventKindUp,
      .key = [[keyCodeToPhysicalKey objectForKey:@(event.keyCode)] intValue],
      .character_size = 0,
  };
  FlutterLogicalKeyEvent logical_events[] = {logicalEvent};
  FlutterKeyEvent flutterEvent = {
      .struct_size = sizeof(FlutterKeyEvent),
      .logical_event_count = 1,
      .logical_events = logical_events,
      .timestamp = 1,
      .kind = event.type == NSEventTypeKeyDown ? kFlutterKeyEventKindDown : kFlutterKeyEventKindUp,
      .key = [[keyCodeToPhysicalKey objectForKey:@(event.keyCode)] intValue],
  };
  [_flutterViewController dispatchFlutterKeyEvent:flutterEvent];
}

#pragma mark - Private

@end
