// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <memory>
#include <string>

#include "FlutterPlatformViews_Internal.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterChannels.h"

@implementation FlutterPlatformViewsController {
  fml::scoped_nsobject<FlutterMethodChannel> _channel;
  std::map<std::string, fml::scoped_nsobject<NSObject<FlutterPlatformViewFactory>>> _factories;
  std::map<long, fml::scoped_nsobject<NSObject<FlutterPlatformView>>> _views;
}

- (instancetype)init:(NSObject<FlutterBinaryMessenger>*)withMessenger {
  if (self = [super init]) {
    _channel.reset([[FlutterMethodChannel alloc]
           initWithName:@"flutter/platform_views"
        binaryMessenger:withMessenger
                  codec:[FlutterStandardMethodCodec sharedInstance]]);
    [_channel.get() setMethodCallHandler:^(FlutterMethodCall* call, FlutterResult result) {
      [self methodCallHandler:call withResult:result];
    }];
  }
  return self;
}

- (void)registerViewFactory:(NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(NSString*)factoryId {
  std::string idString([factoryId UTF8String]);
  NSAssert(_factories.count(idString) == 0,
           ([NSString stringWithFormat:@"Can't register an already registered view factory: %@",
                                       factoryId]));
  _factories[idString] =
      fml::scoped_nsobject<NSObject<FlutterPlatformViewFactory>>([factory retain]);
}

- (void)methodCallHandler:(FlutterMethodCall*)call withResult:(FlutterResult)result {
  if ([[call method] isEqualToString:@"create"]) {
    [self handleCreate:call withResult:result];
  } else if ([[call method] isEqualToString:@"dispose"]) {
    [self handleDispose:call withResult:result];
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (void)handleCreate:(FlutterMethodCall*)call withResult:(FlutterResult)result {
  NSDictionary<NSString*, id>* args = [call arguments];

  long viewId = [args[@"id"] longValue];
  std::string viewType([args[@"viewType"] UTF8String]);

  if (_views[viewId] != nil) {
    result([FlutterError errorWithCode:@"recreating_view"
                               message:@"trying to create an already created view"
                               details:[NSString stringWithFormat:@"view id: '%ld'", viewId]]);
  }

  NSObject<FlutterPlatformViewFactory>* factory = _factories[viewType].get();
  if (factory == nil) {
    result([FlutterError errorWithCode:@"unregistered_view_type"
                               message:@"trying to create a view with an unregistered type"
                               details:[NSString stringWithFormat:@"unregistered view type: '%@'",
                                                                  args[@"viewType"]]]);
    return;
  }

  // TODO(amirh): decode and pass the creation args.
  _views[viewId] =
      fml::scoped_nsobject<NSObject<FlutterPlatformView>>([[factory createWithFrame:CGRectZero
                                                                            andArgs:nil] retain]);
  result(nil);
}

- (void)handleDispose:(FlutterMethodCall*)call withResult:(FlutterResult)result {
  NSDictionary<NSString*, id>* args = [call arguments];
  long viewId = [args[@"id"] longValue];

  if (_views[viewId] == nil) {
    result([FlutterError errorWithCode:@"unknown_view"
                               message:@"trying to dispose an unknown"
                               details:[NSString stringWithFormat:@"view id: '%ld'", viewId]]);
    return;
  }

  [_views[viewId] dispose];
  _views.erase(viewId);
  result(nil);
}

@end
