// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERDARTPROJECT_INTERNAL_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERDARTPROJECT_INTERNAL_H_

#include "flutter/common/settings.h"
#include "flutter/runtime/platform_data.h"
#include "flutter/shell/common/engine.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterDartProject.h"

NS_ASSUME_NONNULL_BEGIN

flutter::Settings FLTDefaultSettingsForBundle(NSBundle* bundle = nil);

@interface FlutterDartProject ()

@property(nonatomic, readonly) BOOL isWideGamutEnabled;

/**
 * This is currently used for *only for tests* to override settings.
 */
- (instancetype)initWithSettings:(const flutter::Settings&)settings;
- (const flutter::Settings&)settings;
- (const flutter::PlatformData)defaultPlatformData;

- (flutter::RunConfiguration)runConfiguration;
- (flutter::RunConfiguration)runConfigurationForEntrypoint:(nullable NSString*)entrypointOrNil;
- (flutter::RunConfiguration)runConfigurationForEntrypoint:(nullable NSString*)entrypointOrNil
                                              libraryOrNil:(nullable NSString*)dartLibraryOrNil;
- (flutter::RunConfiguration)runConfigurationForEntrypoint:(nullable NSString*)entrypointOrNil
                                              libraryOrNil:(nullable NSString*)dartLibraryOrNil
                                            entrypointArgs:
                                                (nullable NSArray<NSString*>*)entrypointArgs;

+ (NSString*)flutterAssetsName:(NSBundle*)bundle;
+ (NSString*)domainNetworkPolicy:(NSDictionary*)appTransportSecurity;
+ (bool)allowsArbitraryLoads:(NSDictionary*)appTransportSecurity;

// Embedder API
/**
 * The path to the Flutter assets directory.
 *
 * In UTF8String format.
 */
@property(nonatomic, readonly, nullable) NSString* assetsPath;

/**
 * The path to the ICU data file.
 *
 * In UTF8String format.
 */
@property(nonatomic, readonly, nullable) NSString* ICUDataPath;

/**
 * The command line arguments array for the engine.
 */
@property(nonatomic, readonly) std::vector<std::string> switches;

/**
 * The callback invoked by the engine in root isolate scope.
 */
@property(nonatomic, nullable) void (*rootIsolateCreateCallback)(void* _Nullable);

@end

NS_ASSUME_NONNULL_END

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTERDARTPROJECT_INTERNAL_H_
