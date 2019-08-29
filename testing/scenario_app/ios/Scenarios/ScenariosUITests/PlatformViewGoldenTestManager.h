// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import "GoldenImage.h"

NS_ASSUME_NONNULL_BEGIN

// Manages a `GoldenPlatformViewTests`.
//
// It creates the correct GoldenImage based on the `identifer`.
// It also generates the correct launchArgs to launch the associate platform view scenario.
@interface PlatformViewGoldenTestManager : NSObject

@property (readonly, strong, nonatomic) GoldenImage *goldenImage;
@property (readonly, strong, nonatomic) NSArray *launchArgs;

// Initilize with identifier.
//
// Crahes if the identifier is not mapped in `launchArgsMap` inside PlatformViewGoldenTestManager.m
- (instancetype)initWithIdentifier:(nonnull NSString *)identifier;

@end

NS_ASSUME_NONNULL_END
