// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"

#if FLUTTER_RUNTIME_MODE == FLUTTER_RUNTIME_MODE_DEBUG
FLUTTER_DARWIN_EXPORT
#endif

/**
 * A proxy object with a weak reference back to the engine.
 *
 * If we make having the |FlutterTextureRegistry| returned by |textures| be the engine itself, it
 * could be problematic, because if the plugin retains the texture registrar there's a cycle.
 */
@interface FlutterTextureRegistryRelay : NSObject <FlutterTextureRegistry>
@property(nonatomic, assign) NSObject<FlutterTextureRegistry>* parent;
- (instancetype)initWithParent:(NSObject<FlutterTextureRegistry>*)parent;
@end
