// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FLUTTER_NOLINT: https://github.com/flutter/flutter/issues/93360

// The purpose of this file is to ensure that the Flutter framework is re-exported through the
// FlutterMacOS framework. Since [FlutterMouseTrackingMode] is from the Flutter framework, it would
// error if the re-exporting was not working.
// The target that uses this file copies the headers to a path that simulates how users would
// actually import the framework outside of the engine source root
#import <FlutterMacOS/FlutterMacOS.h>

@interface TestReExport : NSObject

- (void)test:(FlutterMouseTrackingMode)mode;

@end
