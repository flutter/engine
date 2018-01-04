// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterDartProject_Internal.h"

#include "flutter/common/task_runners.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/switches.h"
#include "flutter/shell/platform/darwin/common/command_line.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"

static blink::Settings DefaultSettingsForProcess() {
  auto command_line = shell::CommandLineFromNSProcessInfo();

  // Settings passed in explicitly via command line arguments take priority.
  auto settings = shell::SettingsFromCommandLine(command_line);

  // The command line arguments may not always be complete. If they aren't, attempt to fill in
  // defaults.

  if (settings.icu_data_path.size() == 0) {
    // Flutter ships the ICU data file in the the bundle of the engine.
    NSBundle* bundle = [NSBundle bundleForClass:[FlutterViewController class]];
    NSString* icuDataPath = [bundle pathForResource:@"icudtl" ofType:@"dat"];
    if (icuDataPath.length > 0) {
      settings.icu_data_path = icuDataPath.UTF8String;
    }
  }

  if (settings.application_library_path.size() == 0) {
    NSString* libraryName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"FLTLibraryPath"];
    NSString* libraryPath = [[NSBundle mainBundle] pathForResource:libraryName ofType:nil];
    if (libraryPath.length > 0) {
      settings.application_library_path = libraryPath.UTF8String;
    }
  }

  return settings;
}

@implementation FlutterDartProject {
  fml::scoped_nsobject<NSBundle> _precompiledDartBundle;
  blink::Settings _settings;
}

#pragma mark - Override base class designated initializers

- (instancetype)init {
  return [self initWithFlutterAssets:nil dartMain:nil packages:nil];
}

#pragma mark - Designated initializers

- (instancetype)initWithPrecompiledDartBundle:(NSBundle*)bundle {
  if (bundle == nil) {
    [self release];
    return nil;
  }

  self = [super init];

  if (self) {
    _precompiledDartBundle.reset([bundle retain]);
    _settings = DefaultSettingsForProcess();
    _settings.application_library_path = _precompiledDartBundle.get().executablePath.UTF8String;
  }

  return self;
}

- (instancetype)initWithFLXArchive:(NSURL*)archiveURL
                          dartMain:(NSURL*)dartMainURL
                          packages:(NSURL*)dartPackages {
  return nil;
}

- (instancetype)initWithFLXArchiveWithScriptSnapshot:(NSURL*)archiveURL {
  return nil;
}

- (instancetype)initWithFlutterAssets:(NSURL*)flutterAssetsURL
                             dartMain:(NSURL*)dartMainURL
                             packages:(NSURL*)dartPackages {
  self = [super init];

  if (self) {
    _settings = DefaultSettingsForProcess();

    if (archiveURL != nil) {
      _settings.flx_path = archiveURL.absoluteString.UTF8String;
    }

    if (dartMainURL != nil) {
      _settings.main_dart_file_path = dartMainURL.absoluteString.UTF8String;
    }

    if (dartPackages != nil) {
      _settings.packages_file_path = dartPackages.absoluteString.UTF8String;
    }
  }

  return self;
}

- (instancetype)initWithFlutterAssetsWithScriptSnapshot:(NSURL*)flutterAssetsURL {
  self = [super init];

  if (self) {
    _settings = DefaultSettingsForProcess();

    _settings.flx_path = archiveURL.absoluteString.UTF8String;
  }

  return self;
}

#pragma mark - Convenience initializers

- (instancetype)initFromDefaultSourceForConfiguration {
  if (blink::DartVM::IsRunningPrecompiledCode()) {
    return
        [self initWithPrecompiledDartBundle:[NSBundle bundleWithPath:@"Frameworks/App.framework"]];
  } else {
    return [self initWithFLXArchive:nil dartMain:nil packages:nil];
  }
}

- (const blink::Settings&)settings {
  return _settings;
}

@end
