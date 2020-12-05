// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <unordered_set>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViews.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

@interface FlutterViewController ()

// The FlutterView for this view controller.
@property(nonatomic, readonly, nullable) FlutterView* flutterView;

// NSDictionary maps strings to FlutterPlatformViewFactorys.
@property(nonnull, nonatomic)
    NSMutableDictionary<NSString*, NSObject<FlutterPlatformViewFactory>*>* factories;

// A map of platform view ids to views.
@property(nonatomic) std::map<int, NSView*> platformViews;

// View ids that are going to be disposed on the next present call.
@property(nonatomic) std::unordered_set<int64_t> platformViewsToDispose;

/**
 * This just returns the NSPasteboard so that it can be mocked in the tests.
 */
@property(nonatomic, readonly, nonnull) NSPasteboard* pasteboard;

/**
 * Platform View Methods.
 */

/**
 * Creates a platform view using the arguments from the provided call.
 * The call's arguments should be castable to an NSMutableDictionary<NSString*, id>*
 * and the dictionary should at least hold one key for "id" that maps to the view id and
 * one key for "viewType" which maps to the view type (string) that was used to register
 * the factory.
 * FlutterResult is updated to contain nil for success or to contain
 * a FlutterError if there is an error.
 */
- (void)onCreate:(nonnull FlutterMethodCall*)call result:(nonnull FlutterResult)result;

/**
 * Disposes a platform view using the arguments from the provided call.
 * The call's arguments should be the Id (castable to NSNumber*) of the platform view
 * that should be disposed.
 * FlutterResult is updated to contain nil for success or a FlutterError if there is an error.
 */
- (void)onDispose:(nonnull FlutterMethodCall*)call result:(nonnull FlutterResult)result;

/**
 * Register a view factory by adding an entry into the factories_ map with key factoryId
 * and value factory.
 */
- (void)registerViewFactory:(nonnull NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(nonnull NSString*)factoryId;

/**
 * Adds a responder for keyboard events. Key up and key down events are forwarded to all added
 * responders.
 */
- (void)addKeyResponder:(nonnull NSResponder*)responder;

/**
 * Removes a responder for keyboard events.
 */
- (void)removeKeyResponder:(nonnull NSResponder*)responder;

@end
