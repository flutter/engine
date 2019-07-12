// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Flutter/Flutter.h>
#import <UIKit/UIKit.h>

@interface AppDelegate : FlutterAppDelegate

@property(nonatomic, strong) FlutterEngine* engine;
@property(nonatomic, strong) UIWindow* window;
@property(nonatomic, strong) FlutterBasicMessageChannel* reloadMessageChannel;

@end
