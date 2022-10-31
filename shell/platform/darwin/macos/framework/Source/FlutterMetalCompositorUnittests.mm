// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>
#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/testing/testing.h"

namespace flutter::testing {
namespace {

FlutterViewProvider* MockViewProvider() {
  FlutterView* viewMock = OCMClassMock([FlutterView class]);
  FlutterMetalRenderBackingStore* backingStoreMock =
      OCMClassMock([FlutterMetalRenderBackingStore class]);
  __block id<MTLTexture> textureMock = OCMProtocolMock(@protocol(MTLTexture));
  OCMStub([backingStoreMock texture]).andReturn(textureMock);

  OCMStub([viewMock backingStoreForSize:CGSize{}])
      .ignoringNonObjectArgs()
      .andDo(^(NSInvocation* invocation) {
        CGSize size;
        [invocation getArgument:&size atIndex:2];
        OCMStub([textureMock width]).andReturn(size.width);
        OCMStub([textureMock height]).andReturn(size.height);
      })
      .andReturn(backingStoreMock);

  FlutterViewProvider* viewProviderMock = OCMStrictClassMock([FlutterViewProvider class]);
  OCMStub([viewProviderMock getView:kFlutterDefaultViewId]).ignoringNonObjectArgs().andReturn(viewMock);
  return viewProviderMock;
}
}  // namespace

TEST(FlutterMetalCompositorTest, TestPresent) {
  std::unique_ptr<flutter::FlutterMetalCompositor> macos_compositor =
      std::make_unique<FlutterMetalCompositor>(
          MockViewProvider(), /*platform_view_controller*/ nullptr, /*mtl_device*/ nullptr);

  bool flag = false;
  macos_compositor->SetPresentCallback([f = &flag](bool has_flutter_content) {
    *f = true;
    return true;
  });

  ASSERT_TRUE(macos_compositor->Present(nil, 0));
  ASSERT_TRUE(flag);
}

TEST(FlutterMetalCompositorTest, TestCreate) {
  std::unique_ptr<flutter::FlutterMetalCompositor> macos_compositor =
      std::make_unique<FlutterMetalCompositor>(
          MockViewProvider(), /*platform_view_controller*/ nullptr, /*mtl_device*/ nullptr);

  FlutterBackingStore backing_store;
  FlutterBackingStoreConfig config;
  config.struct_size = sizeof(FlutterBackingStoreConfig);
  config.size.width = 800;
  config.size.height = 600;
  macos_compositor->CreateBackingStore(&config, &backing_store);

  ASSERT_EQ(backing_store.type, kFlutterBackingStoreTypeMetal);
  ASSERT_NE(backing_store.metal.texture.texture, nil);
  id<MTLTexture> texture = (__bridge id<MTLTexture>)backing_store.metal.texture.texture;
  ASSERT_EQ(texture.width, 800ul);
  ASSERT_EQ(texture.height, 600ul);
}

TEST(FlutterMetalCompositorTest, TestCompositing) {
  std::unique_ptr<flutter::FlutterMetalCompositor> macos_compositor =
      std::make_unique<FlutterMetalCompositor>(
          MockViewProvider(), /*platform_view_controller*/ nullptr, /*mtl_device*/ nullptr);

  FlutterBackingStore backing_store;
  FlutterBackingStoreConfig config;
  config.struct_size = sizeof(FlutterBackingStoreConfig);
  config.size.width = 800;
  config.size.height = 600;
  macos_compositor->CreateBackingStore(&config, &backing_store);

  ASSERT_EQ(backing_store.type, kFlutterBackingStoreTypeMetal);
  ASSERT_NE(backing_store.metal.texture.texture, nil);
  id<MTLTexture> texture = (__bridge id<MTLTexture>)backing_store.metal.texture.texture;
  ASSERT_EQ(texture.width, 800u);
  ASSERT_EQ(texture.height, 600u);
}

}  // namespace flutter::testing
