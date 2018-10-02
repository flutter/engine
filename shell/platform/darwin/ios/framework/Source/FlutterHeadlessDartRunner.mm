// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"

#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterHeadlessDartRunner.h"

#include <memory>

#include "flutter/fml/make_copyable.h"
#include "flutter/fml/message_loop.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/switches.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/platform/darwin/common/command_line.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPlugin.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterDartProject_Internal.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/platform_message_response_darwin.h"
#include "flutter/shell/platform/darwin/ios/platform_view_ios.h"

@implementation FlutterHeadlessDartRunner {
}

- (bool)runWithEntrypointAndLibraryUri:(NSString*)entrypoint libraryUri:(NSString*)uri {
  bool shellCreated = [super runWithEntrypointAndLibraryUri:entrypoint libraryUri:uri];
  if (!shellCreated || entrypoint.length == 0) {
    FML_LOG(ERROR)
        << "FlutterHeadlessDartRunner requires on proper setup of shell and a valid entrypoint.";
    return false;
  }

  FlutterDartProject* project = [[[FlutterDartProject alloc] init] autorelease];

  auto config = project.runConfiguration;
  config.SetEntrypointAndLibrary(entrypoint.UTF8String, uri.UTF8String);

  // Override the default run configuration with the specified entrypoint.
  self.shell.GetTaskRunners().GetUITaskRunner()->PostTask(
      fml::MakeCopyable([engine = self.shell.GetEngine(), config = std::move(config)]() mutable {
        BOOL success = NO;
        FML_LOG(INFO) << "Attempting to launch background engine configuration...";
        if (!engine || engine->Run(std::move(config)) == shell::Engine::RunStatus::Failure) {
          FML_LOG(ERROR) << "Could not launch engine with configuration.";
        } else {
          FML_LOG(INFO) << "Background Isolate successfully started and run.";
          success = YES;
        }
      }));
  return true;
}

- (bool)runWithEntrypoint:(NSString*)entrypoint {
  return [self runWithEntrypointAndLibraryUri:entrypoint libraryUri:nil];
}

@end
