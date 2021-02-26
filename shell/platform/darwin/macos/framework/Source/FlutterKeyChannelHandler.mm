// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <objc/message.h>

#import "FlutterKeyChannelHandler.h"
#import "KeyCodeMap_internal.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterCodecs.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/embedder/embedder.h"

@interface FlutterKeyChannelHandler ()

/**
 * The channel used to communicate with Flutter.
 */
@property(nonatomic) FlutterBasicMessageChannel* channel;

@end

@implementation FlutterKeyChannelHandler

- (nonnull instancetype)initWithChannel:(nonnull FlutterBasicMessageChannel*)channel {
  self = [super init];
  _channel = channel;
  return self;
}

- (void)handleEvent:(NSEvent*)event
             ofType:(NSString*)type
           callback:(FlutterKeyHandlerCallback)callback {
  NSMutableDictionary* keyMessage = [@{
    @"keymap" : @"macos",
    @"type" : type,
    @"keyCode" : @(event.keyCode),
    @"modifiers" : @(event.modifierFlags),
  } mutableCopy];
  // Calling these methods on any other type of event
  // (e.g NSEventTypeFlagsChanged) will raise an exception.
  if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
    keyMessage[@"characters"] = event.characters;
    keyMessage[@"charactersIgnoringModifiers"] = event.charactersIgnoringModifiers;
  }
  [_channel sendMessage:keyMessage
                  reply:^(id reply) {
                    if (!reply) {
                      return callback(true);
                    }
                    // Only propagate the event to other responders if the framework didn't handle
                    // it.
                    callback([[reply valueForKey:@"handled"] boolValue]);
                  }];
}

#pragma mark - Private

@end
