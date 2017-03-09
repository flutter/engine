// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERSTANDARDCODECINTERNAL_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERSTANDARDCODECINTERNAL_H_

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterCodecs.h"

namespace shell {
  const UInt8 kNIL = 0;
  const UInt8 kTRUE = 1;
  const UInt8 kFALSE = 2;
  const UInt8 kINT32 = 3;
  const UInt8 kINT64 = 4;
  const UInt8 kINTHEX = 5;
  const UInt8 kFLOAT64 = 6;
  const UInt8 kSTRING = 7;
  const UInt8 kBYTE_ARRAY = 8;
  const UInt8 kINT32_ARRAY = 9;
  const UInt8 kINT64_ARRAY = 10;
  const UInt8 kFLOAT64_ARRAY = 11;
  const UInt8 kLIST = 12;
  const UInt8 kMAP = 13;
}

@interface FlutterStandardWriter : NSObject
+ (instancetype) withData:(NSMutableData*)data;
- (void)writeByte:(UInt8)value;
- (void)writeValue:(id)value;
@end

@interface FlutterStandardReader: NSObject
+ (instancetype) withData:(NSData*)data;
- (BOOL) hasMore;
- (id) readValue;
@end

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERSTANDARDCODECINTERNAL_H_
