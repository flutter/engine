// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "FlutterWindowPlugin.h"
#import "FlutterEngine_Internal.h"

#include <map>

static NSString* const kChannelName = @"flutter/window";

@interface FlutterWindowPlugin ()
- (instancetype)initWithChannel:(FlutterMethodChannel*)channel engine:(FlutterEngine*)engine;
@end

@implementation FlutterWindowPlugin {
  FlutterMethodChannel* _channel;
  FlutterEngine* _engine;
}

#pragma mark - Private Methods

- (instancetype)initWithChannel:(FlutterMethodChannel*)channel engine:(FlutterEngine*)engine {
  self = [super init];
  if (self) {
    _channel = channel;
    _engine = engine;
  }
  return self;
}

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  NSLog(@">>> method %@", call.method);
  if ([call.method isEqualToString:@"new"]) {
    // dispatch_async(dispatch_get_main_queue(), ^{
    NSRect graphicsRect = NSMakeRect(100.0, 350.0, 844.0, 626.0);
    NSWindow* myWindow = [[NSWindow alloc]
        initWithContentRect:graphicsRect
                  styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask |
                            NSWindowStyleMaskResizable
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [myWindow setTitle:@"Test Test"];

    FlutterViewController* controller = [[FlutterViewController alloc] initWithEngine:_engine
                                                                              nibName:nil
    [myWindow setContentViewController:controller];
    [myWindow setFrame:graphicsRect display:YES];

    // [myWindow setDelegate:*myView ];       // set window's delegate
    [myWindow makeKeyAndOrderFront:nil];  // display window
    [_engine updateWindowMetrics:controller.flutterView id:controller.id];
    NSLog(@"New view ID: %@", @(controller.id));
    result(@(controller.id));
    // });
  }
}

#pragma mark - Public Class Methods

+ (void)registerWithRegistrar:(nonnull id<FlutterPluginRegistrar>)registrar
                       engine:(nonnull FlutterEngine*)engine {
  FlutterMethodChannel* channel = [FlutterMethodChannel methodChannelWithName:kChannelName
                                                              binaryMessenger:registrar.messenger];
  FlutterWindowPlugin* instance = [[FlutterWindowPlugin alloc] initWithChannel:channel
                                                                        engine:engine];
  [registrar addMethodCallDelegate:instance channel:channel];
}

+ (void)registerWithRegistrar:(nonnull id<FlutterPluginRegistrar>)registrar {
  NSLog(@"ERROR CALL TO registerWithRegistrar");
}

@end
