// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "PlatformViewGoldenTestManager.h"

@interface PlatformViewGoldenTestManager ()

@property(readwrite, strong, nonatomic) GoldenImage* goldenImage;
@property(readwrite, strong, nonatomic) NSArray* launchArgs;

// Maps the `identifier` to the launch args for the platform view test associated with this
// `GoldenImage`
@property(class, nonatomic, strong) NSDictionary* launchArgsMap;

@end

@implementation PlatformViewGoldenTestManager

static NSDictionary* _launchArgsMap;

+ (void)initialize {
  if (self == [PlatformViewGoldenTestManager class]) {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      [self setLaunchArgsMap:@{
        @"platform_view" : @[ @"--platform-view" ],
        @"platform_view_cliprect" : @[ @"--platform-view-cliprect" ],
        @"platform_view_cliprrect" : @[ @"--platform-view-cliprrect" ],
        @"platform_view_clippath" : @[ @"--platform-view-clippath" ],
        @"platform_view_transform" : @[ @"--platform-view-transform" ],
        @"platform_view_opacity" : @[ @"--platform-view-opacity" ],
      }];
    });
  }
}

+ (NSDictionary*)launchArgsMap {
  return _launchArgsMap;
}

+ (void)setLaunchArgsMap:(NSDictionary*)launchArgsMap {
  _launchArgsMap = launchArgsMap;
}

- (instancetype)initWithIdentifier:(NSString*)identifier {
  self = [super init];
  if (self) {
    _launchArgs = [PlatformViewGoldenTestManager launchArgsMap][identifier];
    NSString* prefix = [NSString stringWithFormat:@"golden_%@_", identifier];
    _goldenImage = [[GoldenImage alloc] initWithGoldenNamePrefix:prefix];
  }
  return self;
}

@end
