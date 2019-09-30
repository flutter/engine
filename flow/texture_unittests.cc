// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/texture.h"
#include "gtest/gtest.h"

//namespace flutter {
//namespace testing {
//
//class MockTexture : public flutter::Texture {
// public:
//  MockTexture();
//
//  ~MockTexture() override = default;
//
//  bool unregistered;
//
//  // Called from GPU thread.
//  void Paint(SkCanvas& canvas, const SkRect& bounds, bool freeze, GrContext* context) override {
//
//  }
//
//  void OnGrContextCreated() override {
//
//  }
//
//  void OnGrContextDestroyed() override {
//
//  }
//
//  void MarkNewFrameAvailable() override {
//
//  }
//
//  void OnUnregistered() override {
//    unregistered = true;
//  }
//};
//
//TEST(TextureRegistry, UnregisterTextureCallbackTriggered) {
//  TextureRegistry textureRegistry;
//  std::shared_ptr<MockTexture> mockTexture = std::make_shared<mockTexture>();
//  textureRegistry.RegisterTexture(mockTexture);
//  textureRegistry.UnregisterTexture(0);
//  ASSERT_TRUE(mockTexture->unregistered);
//}
//
//}  // namespace flutter
