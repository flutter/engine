// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterResourcePlugin.h"
#include "flutter/fml/logging.h"

#include <Foundation/Foundation.h>
#include <UIKit/UIKit.h>

@implementation FlutterResourcePlugin

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result {
  NSString* method = call.method;
  id args = call.arguments;
  if ([method isEqualToString:@"SystemImage.load"]) {
    result([self loadSystemImage:args]);
  } else {
    result(FlutterMethodNotImplemented);
  }
}

- (NSDictionary*)loadSystemImage:(NSDictionary*)args {
  if (@available(ios 13.0, *)) {
    NSString* systemImageName = args[@"name"];
    NSNumber* size = args[@"size"];
    NSNumber* weight = args[@"weight"];

    UIImageSymbolWeight symbolWeight;
    if (weight != (id)[NSNull null]) {
      switch ([weight intValue]) {
        case 100:
          symbolWeight = UIImageSymbolWeightUltraLight;
          break;
        case 200:
          symbolWeight = UIImageSymbolWeightThin;
          break;
        case 300:
          symbolWeight = UIImageSymbolWeightLight;
          break;
        case 400:
          symbolWeight = UIImageSymbolWeightRegular;
          break;
        case 500:
          symbolWeight = UIImageSymbolWeightMedium;
          break;
        case 600:
          symbolWeight = UIImageSymbolWeightSemibold;
          break;
        case 700:
          symbolWeight = UIImageSymbolWeightBold;
          break;
        case 800:
          symbolWeight = UIImageSymbolWeightHeavy;
          break;
        case 900:
          symbolWeight = UIImageSymbolWeightBlack;
          break;
      }
    }

    UIImage* image;

    if (size == (id)[NSNull null]) {
      // If you ask without a size, it will by default return an optically 18pt icon with a 21pt
      // canvas.
      image = [UIImage systemImageNamed:systemImageName];
    } else if (symbolWeight == UIImageSymbolWeightUnspecified) {
      image = [UIImage systemImageNamed:systemImageName
                      withConfiguration:[UIImageSymbolConfiguration
                                            configurationWithPointSize:[size intValue]]];
    } else {
      image = [UIImage
           systemImageNamed:systemImageName
          withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:[size intValue]
                                                                            weight:symbolWeight]];
    }
    NSData* data = UIImagePNGRepresentation(image);
    if (data) {
      return @{
        @"scale" : @(image.scale),
        @"data" : [FlutterStandardTypedData typedDataWithBytes:data],
      };
    } else {
      // The amount of available icons between iOS 13 and 14 are not the same. There will be
      // more misses in iOS 13.
      return nil;
    }
  } else {
    // We only support loading system images in iOS 13+.
    return nil;
  }
}

@end
