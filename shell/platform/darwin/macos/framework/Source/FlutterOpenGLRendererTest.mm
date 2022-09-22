// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/common/framework/Source/FlutterDartProject_Internal.h"
#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEmebedderEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterOpenGLRenderer.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/testing/testing.h"

// MOCK_ENGINE_PROC is leaky by design
// NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)

@interface TestOpenGLEngine : FlutterEmbedderEngine

@property(nonatomic, readwrite) id<FlutterRenderer> renderer;

- (nullable instancetype)initWithGLRenderer;

@end

@implementation TestOpenGLEngine

@synthesize renderer;

- (nullable instancetype)initWithGLRenderer {
  NSString* fixtures = @(flutter::testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  renderer = [[FlutterOpenGLRenderer alloc] init];
  self = [self initWithrRenderer:renderer];
  return self;
}

@end

namespace flutter::testing {

TEST(FlutterOpenGLRenderer, RegisterExternalTexture) {
  FlutterEmebedderEngine** engine = [[TestOpenGLEngine alloc] initWithGLRenderer];
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  engine.embedderAPI.RegisterExternalTexture =
      MOCK_ENGINE_PROC(RegisterExternalTexture, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, reinterpret_cast<int64_t>(flutterTexture));
        return kSuccess;
      });

  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  [openGLRenderer registerTexture:flutterTexture];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

TEST(FlutterOpenGLRenderer, UnregisterExternalTexture) {
  FlutterEmebedderEngine** engine = [[TestOpenGLEngine alloc] initWithGLRenderer];
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  int64_t registeredTextureId = [openGLRenderer registerTexture:flutterTexture];
  engine.embedderAPI.UnregisterExternalTexture =
      MOCK_ENGINE_PROC(UnregisterExternalTexture, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, registeredTextureId);
        return kSuccess;
      });

  [openGLRenderer unregisterTexture:registeredTextureId];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

TEST(FlutterOpenGLRenderer, MarkExternalTextureFrameAvailable) {
  FlutterEmebedderEngine** engine = [[TestOpenGLEngine alloc] initWithGLRenderer];
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  int64_t registeredTextureId = [openGLRenderer registerTexture:flutterTexture];
  engine.embedderAPI.MarkExternalTextureFrameAvailable = MOCK_ENGINE_PROC(
      MarkExternalTextureFrameAvailable, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, registeredTextureId);
        return kSuccess;
      });

  [openGLRenderer textureFrameAvailable:registeredTextureId];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

TEST(FlutterOpenGLRenderer, PresetDelegatesToFlutterView) {
  FlutterEmebedderEngine** engine = [[TestOpenGLEngine alloc] initWithGLRenderer];
  FlutterOpenGLRenderer* renderer = engine.renderer;
  id mockFlutterView = OCMClassMock([FlutterView class]);
  [(FlutterView*)[mockFlutterView expect] present];
  [renderer setFlutterView:mockFlutterView];
  [renderer openGLContext];
  [renderer present];
}

TEST(FlutterOpenGLRenderer, FBOReturnedByFlutterView) {
  FlutterEmebedderEngine** engine = [[TestOpenGLEngine alloc] initWithGLRenderer];
  FlutterOpenGLRenderer* renderer = [engine.renderer;
  id mockFlutterView = OCMClassMock([FlutterView class]);
  FlutterFrameInfo frameInfo;
  frameInfo.struct_size = sizeof(FlutterFrameInfo);
  FlutterUIntSize dimensions;
  dimensions.width = 100;
  dimensions.height = 200;
  frameInfo.size = dimensions;
  CGSize size = CGSizeMake(dimensions.width, dimensions.height);
  [[mockFlutterView expect] backingStoreForSize:size];
  [renderer setFlutterView:mockFlutterView];
  [renderer openGLContext];
  [renderer fboForFrameInfo:&frameInfo];
}

}  // namespace flutter::testing

// NOLINTEND(clang-analyzer-core.StackAddressEscape)
