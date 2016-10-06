// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/platform_service_ios.h"

#include "base/mac/scoped_nsautorelease_pool.h"
#include "lib/ftl/logging.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

namespace shell {

namespace {

void Vibrate(NSMutableDictionary* result) {
  AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
}

void LaunchURL(NSString* urlString, NSMutableDictionary* result) {
  NSURL* url = [NSURL URLWithString:urlString];

  UIApplication* application = [UIApplication sharedApplication];

  bool launched = [application canOpenURL:url] && [application openURL:url];

  [result setObject:(launched ? @"true" : @"false") forKey:@"launched"];
}

void GetDirectoryOfType(NSString* pathType, NSMutableDictionary* result) {
  NSSearchPathDirectory dir;
  if ([pathType isEqual:@"temporary"]) {
    dir = NSCachesDirectory;
  } else if ([pathType isEqual:@"documents"]) {
    dir = NSDocumentDirectory;
  } else {
    FTL_LOG(ERROR) << "Unsupported path type " << [pathType UTF8String];
    return;
  }
  NSArray* paths =
      NSSearchPathForDirectoriesInDomains(dir, NSUserDomainMask, YES);

  if (paths.count == 0) {
    return;
  }

  [result setObject:paths.firstObject forKey:@"path"];
}

}  // namespace

PlatformServiceIOS::PlatformServiceIOS() = default;
PlatformServiceIOS::~PlatformServiceIOS() = default;

void PlatformServiceIOS::Process(std::string data,
                                 std::function<void(std::string)> callback) {
  base::mac::ScopedNSAutoreleasePool pool;

  NSError* err = nil;
  NSString* dataAsNSString = [NSString stringWithUTF8String:data.c_str()];
  NSData* dataAsNSdata =
      [dataAsNSString dataUsingEncoding:NSUTF8StringEncoding];
  NSDictionary* dict = [NSJSONSerialization JSONObjectWithData:dataAsNSdata
                                                       options:0
                                                         error:&err];
  if (err) {
    FTL_LOG(ERROR) << "Unable to parse json " << data << err;
    return;
  }

  NSString* type = dict[@"type"];
  if (!type) {
    FTL_LOG(ERROR) << "No type in data " << data << err;
    return;
  }

  NSMutableDictionary* result = [[NSMutableDictionary alloc] init];

  if ([type isEqual:@"path_provider"]) {
    NSString* pathType = dict[@"path_type"];
    GetDirectoryOfType(pathType, result);
  } else if ([type isEqual:@"url_launcher"]) {
    NSString* url = dict[@"url"];
    LaunchURL(url, result);
  } else if ([type isEqual:@"vibrate"]) {
    Vibrate(result);
  } else {
    FTL_LOG(ERROR) << "Unsupported type " << std::string([type UTF8String]);
    return;
  }
  NSData* resultAsNSData =
      [NSJSONSerialization dataWithJSONObject:result options:0 error:&err];
  if (err) {
    FTL_LOG(ERROR) << "Unable to serialize result " << err;
    return;
  }
  NSString* resultAsNSString =
      [[NSString alloc] initWithData:resultAsNSData
                            encoding:NSUTF8StringEncoding];
  callback([resultAsNSString UTF8String]);
}

}  // namespace shell
