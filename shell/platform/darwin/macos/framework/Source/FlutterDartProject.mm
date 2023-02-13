// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>
#include "flutter/shell/platform/common/engine_switches.h"

#import "flutter/shell/platform/darwin/common/framework/Source/FlutterBaseDartProject_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterDartProject.h"

@implementation FlutterDartProject

- (std::vector<std::string>)switches {
  std::vector<std::string> arguments = [super switches];
  if (self.enableMirrors) {
    arguments.push_back("--dart-flags=--enable_mirrors=true");
  }
  return arguments;
}

@end
