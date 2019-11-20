// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/texture.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

class MockTexture : public Texture {
 public:
  MockTexture(int64_t textureId) : Texture(textureId) {}
  ~MockTexture() override = default;

  // Called from GPU thread.
  void Paint(SkCanvas& canvas,
             const SkRect& bounds,
             bool freeze,
             GrContext* context) override {}

  void OnGrContextCreated() override { gr_context_created_ = true; }
  void OnGrContextDestroyed() override { gr_context_destroyed_ = true; }
  void MarkNewFrameAvailable() override {}
  void OnTextureUnregistered() override { unregistered_ = true; }

  bool gr_context_created() { return gr_context_created_; }
  bool gr_context_destroyed() { return gr_context_destroyed_; }
  bool unregistered() { return unregistered_; }

 private:
  bool gr_context_created_ = false;
  bool gr_context_destroyed_ = false;
  bool unregistered_ = false;
};

class TextureRegistryTest : public ::testing::Test {
 public:
  TextureRegistryTest() = default;
  ~TextureRegistryTest() override = default;

  TextureRegistry& registry() { return registry_; }

 private:
  TextureRegistry registry_;
};

TEST_F(TextureRegistryTest, UnregisterTextureCallbackTriggered) {
  std::shared_ptr<MockTexture> mock_texture1 = std::make_shared<MockTexture>(0);
  std::shared_ptr<MockTexture> mock_texture2 = std::make_shared<MockTexture>(1);

  registry().RegisterTexture(mock_texture1);
  registry().RegisterTexture(mock_texture2);
  ASSERT_EQ(registry().GetTexture(0), mock_texture1);
  ASSERT_EQ(registry().GetTexture(1), mock_texture2);
  ASSERT_FALSE(mock_texture1->unregistered());
  ASSERT_FALSE(mock_texture2->unregistered());

  registry().UnregisterTexture(0);
  ASSERT_EQ(registry().GetTexture(0), nullptr);
  ASSERT_TRUE(mock_texture1->unregistered());
  ASSERT_FALSE(mock_texture2->unregistered());

  registry().UnregisterTexture(1);
  ASSERT_EQ(registry().GetTexture(1), nullptr);
  ASSERT_TRUE(mock_texture1->unregistered());
  ASSERT_TRUE(mock_texture2->unregistered());
}

TEST_F(TextureRegistryTest, GrContextCallbackTriggered) {
  std::shared_ptr<MockTexture> mock_texture1 = std::make_shared<MockTexture>(0);
  std::shared_ptr<MockTexture> mock_texture2 = std::make_shared<MockTexture>(1);

  registry().RegisterTexture(mock_texture1);
  registry().RegisterTexture(mock_texture2);
  ASSERT_FALSE(mock_texture1->gr_context_created());
  ASSERT_FALSE(mock_texture2->gr_context_created());
  ASSERT_FALSE(mock_texture1->gr_context_destroyed());
  ASSERT_FALSE(mock_texture2->gr_context_destroyed());

  registry().OnGrContextCreated();
  ASSERT_TRUE(mock_texture1->gr_context_created());
  ASSERT_TRUE(mock_texture2->gr_context_created());

  registry().UnregisterTexture(0);
  registry().OnGrContextDestroyed();
  ASSERT_FALSE(mock_texture1->gr_context_destroyed());
  ASSERT_TRUE(mock_texture2->gr_context_created());
}

TEST_F(TextureRegistryTest, RegisterTextureTwice) {
  std::shared_ptr<MockTexture> mock_texture1 = std::make_shared<MockTexture>(0);
  std::shared_ptr<MockTexture> mock_texture2 = std::make_shared<MockTexture>(0);

  registry().RegisterTexture(mock_texture1);
  ASSERT_EQ(registry().GetTexture(0), mock_texture1);
  registry().RegisterTexture(mock_texture2);
  ASSERT_EQ(registry().GetTexture(0), mock_texture2);
  ASSERT_FALSE(mock_texture1->unregistered());
  ASSERT_FALSE(mock_texture2->unregistered());

  registry().UnregisterTexture(0);
  ASSERT_EQ(registry().GetTexture(0), nullptr);
  ASSERT_FALSE(mock_texture1->unregistered());
  ASSERT_TRUE(mock_texture2->unregistered());
}

TEST_F(TextureRegistryTest, ReuseSameTextureSlot) {
  std::shared_ptr<MockTexture> mock_texture1 = std::make_shared<MockTexture>(0);
  std::shared_ptr<MockTexture> mock_texture2 = std::make_shared<MockTexture>(0);

  registry().RegisterTexture(mock_texture1);
  ASSERT_EQ(registry().GetTexture(0), mock_texture1);

  registry().UnregisterTexture(0);
  ASSERT_EQ(registry().GetTexture(0), nullptr);
  ASSERT_TRUE(mock_texture1->unregistered());
  ASSERT_FALSE(mock_texture2->unregistered());

  registry().RegisterTexture(mock_texture2);
  ASSERT_EQ(registry().GetTexture(0), mock_texture2);

  registry().UnregisterTexture(0);
  ASSERT_EQ(registry().GetTexture(0), nullptr);
  ASSERT_TRUE(mock_texture1->unregistered());
  ASSERT_TRUE(mock_texture2->unregistered());
}

}  // namespace testing
}  // namespace flutter
