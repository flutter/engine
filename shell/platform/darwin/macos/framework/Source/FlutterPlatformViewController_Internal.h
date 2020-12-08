// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import "FlutterChannels.h"

#include <map>
#include <unordered_set>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViews.h"

@interface FlutterPlatformViewController : NSViewController
@end

@interface FlutterPlatformViewController ()

// NSDictionary maps strings to FlutterPlatformViewFactorys.
@property(nonnull, nonatomic)
    NSMutableDictionary<NSString*, NSObject<FlutterPlatformViewFactory>*>* platformViewFactories;

// A map of platform view ids to views.
@property(nonatomic) std::map<int, NSView*> platformViews;

// View ids that are going to be disposed on the next present call.
@property(nonatomic) std::unordered_set<int64_t> platformViewsToDispose;

/**
 * Creates a platform view of viewType with viewId.
 * FlutterResult is updated to contain nil for success or to contain
 * a FlutterError if there is an error.
 */
- (void)onCreateWithViewId:(int64_t)viewId
                  viewType:(nonnull NSString*)viewType
                    result:(nonnull FlutterResult)result;

/**
 * Disposes the platform view with id viewId.
 * FlutterResult is updated to contain nil for success or a FlutterError if there is an error.
 */
- (void)onDisposeWithViewId:(int64_t)viewId result:(nonnull FlutterResult)result;

/**
 * Register a view factory by adding an entry into the platformViewFactories map with key factoryId
 * and value factory.
 */
- (void)registerViewFactory:(nonnull NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(nonnull NSString*)factoryId;

- (void)handleMethodCall:(nonnull FlutterMethodCall*)call result:(nonnull FlutterResult)result;

/**
 * Remove platform views who's ids are in the platformViewsToDispose set.
 * Called before a new frame is presented.
 */
- (void)disposePlatformViews;

@end
